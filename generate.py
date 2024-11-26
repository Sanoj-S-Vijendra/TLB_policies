import random

def generate_sequential_access(n):
    """Generate a sequential access pattern."""
    return [i % m for i in range(n)]

def generate_random_access(n):
    """Generate a random access pattern."""
    return [random.randint(0, m-1) for _ in range(n)]

def generate_strided_access(n, stride):
    """Generate a strided access pattern."""
    return [(i * stride) % m for i in range(n)]

def generate_temporal_locality(n, working_set_size):
    """Generate an access pattern with temporal locality."""
    working_set = [random.randint(0, m-1) for _ in range(working_set_size)]
    return [random.choice(working_set) for _ in range(n)]

def generate_spatial_locality(n, block_size):
    """Generate an access pattern with spatial locality."""
    addresses = []
    for _ in range(n // block_size):
        base_address = random.randint(0, m - (block_size + 1))
        addresses.extend(range(base_address, base_address + block_size))
    return addresses[:n]

def save_to_file(filename, addresses):
    """Save the generated addresses to a file."""
    with open(filename, "w") as file:
        for addr in addresses:
            file.write(f"{addr}\n")
    print(f"Saved {len(addresses)} addresses to {filename}")

# Generate workloads
n = 100000  # Number of addresses
m = 1024
workloads = {
    "sequential": generate_sequential_access(n),
    "random": generate_random_access(n),
    "strided": generate_strided_access(n, stride=4),
    "temporal_locality": generate_temporal_locality(n, working_set_size=16),
    "spatial_locality": generate_spatial_locality(n, block_size=8),
}

# Save each workload to its own file
for name, addresses in workloads.items():
    save_to_file(f"{name}.txt", addresses)
