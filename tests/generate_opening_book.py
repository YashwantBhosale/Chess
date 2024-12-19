def parse_opening_book(filename, output_filename):
    try:
        with open(filename, "r") as f_in, open(output_filename, "w") as f_out:
            fen = ""
            for line in f_in:
                line = line.strip()

                if line.startswith("pos "):
                    fen = line[4:]

                elif line and fen:
                    try:
                        move, _ = line.split()
                        turn = fen.split()[1]
                        f_out.write(f"{fen},{turn},{move}\n")
                        fen = ""
                    except ValueError:
                        print(f"Skipping line due to format: {line}")  # Handle bad format

    except FileNotFoundError:
        print(f"File {filename} not found!")

if __name__ == "__main__":
    parse_opening_book("Book.txt", "book.csv")
    print("Opening book parsed successfully!")
