import random


def generate_random_csv_data(rows: int, cols: int) -> list[list[str]]:
    """Generates a list of lists with random float values as strings."""
    return [
        [f"{random.uniform(-20, 1050):.4f}" for _ in range(cols)] for _ in range(rows)
    ]


print(generate_random_csv_data(10, 16))
