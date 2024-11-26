#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <chrono>
#include <cstring>
#include <random>

using namespace std;

const char* files[] = {"sequential.txt","random.txt","strided.txt","temporal_locality.txt","spatial_locality.txt"};

#define VIRTUAL_MEMORY_SIZE 1024    //virtual address values range(0,1023)
#define BLOCK_SIZE 8               //size of tlb cache block
#define TLB1_SIZE 8 
#define TLB2_SIZE 32

using namespace std;

class Virtual {
public:
    bool empty;
    Virtual() : empty(true) { 
    }
};

class VirtualMemory {
public:
    Virtual virtual_space[VIRTUAL_MEMORY_SIZE];
    VirtualMemory() {
    }
    ~VirtualMemory() {
    }

    void insert(int address) {  
        virtual_space[address].empty = false;
    }

    int find(int address){
        if(virtual_space[address].empty ){ 
            return 0;           
        }
        return 1;
    }
};


class block {
public:
    int line[BLOCK_SIZE];
    int age;
    bool empty;
    block():age(0), empty(true)  {
        for (int i = 0; i < BLOCK_SIZE; ++i) {
            line[i] = 0; 
        }
    }
};

class TranslationLookAsideBuffer {
public:
    //int inclusivity;
    int level;
    int size;
    TranslationLookAsideBuffer * high_level , * low_level ;
    int max_age;    
    vector<block> cache;

    TranslationLookAsideBuffer(int l) : max_age(0) { 
        assert(l == 1 || l == 2);
        if(l == 1){size = TLB1_SIZE;}
        else{size = TLB2_SIZE;}

        cache = vector<block>(size);
        high_level = nullptr;                                         //2 is lower than 1---------------------------------------------
        low_level = nullptr;
        level = l;
    }

    ~TranslationLookAsideBuffer() {
    }

    int get(int address) { 
        int a = address / BLOCK_SIZE;
        for (int i=0;i<size;i++) {
            if (cache[i].empty == false && cache[i].line[0] == a*BLOCK_SIZE){              
                max_age++;
                cache[i].age = max_age;
                return 1;
            }
        }
        return 0;
    }

    void insert(int address, int inclusivity) { 
        int a = address / BLOCK_SIZE;
        for (int i=0;i<size;i++) {
            if (cache[i].empty) {         // empty cell found ...
                for(int j = 0; j < BLOCK_SIZE; j++){
                    cache[i].line[j] = a*BLOCK_SIZE+j;
                }
                max_age++;
                cache[i].age = max_age;
                cache[i].empty = false;
                return;
            }
        }
        int min_age = cache[0].age;
        int position = 0;
        for (int i=1;i<size;i++) {
            if (cache[i].age < min_age) {
                min_age = cache[i].age;
                position = i;
            }
        }
        if (inclusivity == 0){
            if(level == 2){
               high_level -> remove(cache[position].line[0]);
            }
            if(level == 1){
                low_level -> insert(cache[position].line[0], inclusivity);
            }
        }
        if (inclusivity == 1){
            if(level == 1){
                low_level -> insert(cache[position].line[0], inclusivity);
            }
        }
        for(int j = 0; j < BLOCK_SIZE; j++){
            cache[position].line[j] = a*BLOCK_SIZE+j;
        }
        max_age++;
        cache[position].age = max_age;
        cache[position].empty = false;
    }

    void remove(int address){
        int a = address / BLOCK_SIZE;
        for (int i=0;i<size;i++) {
            if (cache[i].line[0] == a*BLOCK_SIZE){             
                cache[i].empty = true;
                return;
            }
        }
    }
};

class TLB {
public:
    //int inclusivity;                      // 0 for inclusive 1 for exclusive -1 for non-inclusive---------------------------------
    TranslationLookAsideBuffer tlbL1, tlbL2;

    TLB() 
        : tlbL1(1), 
          tlbL2(2) {
        tlbL1.low_level = &tlbL2;
        tlbL2.high_level = &tlbL1;
    }

    ~TLB(){
    }

    void insert(int address, int inclusivity){
        if(inclusivity == 1){
            tlbL1.insert(address,inclusivity);
        }
        else {
            tlbL1.insert(address,inclusivity);
            tlbL2.insert(address,inclusivity);
        }
    }

    int get(int address){
        if(tlbL1.get(address))return 1;
        if(tlbL2.get(address))return 2;
        return 0;
    }
};

class Simulation {
public:
    VirtualMemory virtualMemory;
    TLB tlb;
    char file[25];
    int incl;

    Simulation(int inclusivity_value)
        : incl(inclusivity_value) {
    }

    ~Simulation() {
    }

    void run(int iter, int a, int b) {
        strcpy(file,files[iter]);
        ifstream in(file);     //File open for reading
        int temp;                       //for the virtal address from address.txt
        int counter = 0;                //counter for all integers i read
        int page;                       //the page i calculate from virtual address
        int offset;                     //the offset i calculate from the virtual address
        int tlbL1_hits = 0;
        int tlbL2_hits = 0;
        int vm_hits = 0;
        int tlb_misses = 0;
        int pf = 0;
        char byte;

        cout << "\nSimulation is starting ... \n" << endl;

        if (!in) {
            cerr << "Cannot open file " << file << endl;
            return;
        } else {
            cout << "File " << file << " opened successfully !!!\n" << endl;
        }
        while ((in>>temp) && !in.eof()) {    
            counter ++;
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distr(1,1000);
            int rand_num = distr(gen);

            if (virtualMemory.find(temp)) { 
                if (tlb.get(temp)) {  
                    if(tlb.get(temp) == 1){tlbL1_hits++;}
                    else {tlbL2_hits++;}
                } 
                else{
                    if(incl == -1 || incl == 1 || incl == 0)tlb.insert(temp,incl);
                    else if(incl == 2) {
                        if(rand_num <= a*1000){tlb.insert(temp,-1);}
                        else if(rand_num <= b*1000){tlb.insert(temp,0);}
                        else {tlb.insert(temp,1);}
                    }
                    else {
                        cout<<"Wrong inclusivity value given";
                        abort();
                    }   
                    vm_hits++;
                    tlb_misses++;
                }
            }
            else { 
                virtualMemory.insert(temp);  
                if(incl == -1 || incl == 1 || incl == 0)tlb.insert(temp,incl);
                else if (incl == 2) {
                    if(rand_num <= a*1000){tlb.insert(temp,-1);}
                    else if(rand_num <= b*1000){tlb.insert(temp,0);}
                    else {tlb.insert(temp,1);}
                }  
                else {
                    cout<<"Wrong inclusivity value given";
                    abort();
                    }     
                pf++; 
            }
        }

        in.close();

        cout << "\n----------------------------------------- Printing Statistics ----------------------------------------\n" << endl;
        cout << "       References : " <<counter <<"\n"<<endl;
        cout << "       Page faults: " << setw(4) << pf << " (" << (pf*100.0/counter) << "%)\n" << endl;
        cout << "       TLB1 hits  : " << setw(4) << tlbL1_hits << " (" << (tlbL1_hits*100.0/(counter-pf)) << "%)\n" << endl;
        if(tlbL1_hits != counter)
            cout << "       TLB2 hits  : " << setw(4) << tlbL2_hits << " (" << (tlbL2_hits*100.0/(counter-tlbL1_hits-pf)) << "%)\n" << endl;
        cout << "       Total TLB  hits: " << setw(4) << tlbL1_hits+tlbL2_hits << " (" << ((tlbL1_hits+tlbL2_hits)*100.0/(counter-pf)) << "%)\n" << endl;
        cout << "       TLB misses : " << setw(4) << tlb_misses << " (" << (tlb_misses*100.0/(counter-pf)) << "%)\n" << endl;
        cout << "       VM hits    : " << setw(4) << vm_hits << " (" << (vm_hits*100.0/counter) << "%)\n" << endl;
    }
};

int main() {
    int inclusivity;
    cout<<"Enter inclusivity value (-1: non-inclusive, 0: inclusive, 1: exclusive and 2: similar to BIP): ";
    cin >> inclusivity;
    double a=0.2,b=0.4,c=1,d;
    if(inclusivity == 2) {
        cout<<"Enter fractions of non-inclusive, inclusive and exclusive values respectively (default 0.2, 0.2, 0.6): ";
        cin>>a>>b>>c;
        d = a+b+c;
        a = a/d;
        b = a + b/d;
        c = b + c/d;
    }
    for(int i=0; i < 5; i++){
        Simulation simulation = Simulation(inclusivity);
        // auto start = std::chrono::high_resolution_clock::now();
        simulation.run(i,a,b);
        // auto end = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> duration = end - start;
        // cout << "Execution time: " << duration.count() << " seconds" << endl;
    }
    return 0;
}
