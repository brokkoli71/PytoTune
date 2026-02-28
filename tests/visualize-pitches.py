import matplotlib.pyplot as plt
import os

def visualize_pitches(file_path):
    # Check if file exists to avoid immediate crash
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' not found. Current directory: {os.getcwd()}")
        return

    try:
        # Read the file and parse floats
        with open(file_path, 'r') as f:
            pitches = []
            for line_num, line in enumerate(f, 1):
                clean_line = line.strip()
                if clean_line:
                    try:
                        pitches.append(float(clean_line))
                    except ValueError:
                        print(f"Warning: Could not parse line {line_num}: '{clean_line}'")

        if not pitches:
            print("No valid pitch data found in file.")
            return

        # Create the plot
        plt.figure(figsize=(12, 6))
        plt.plot(pitches, marker='o', markersize=2, linestyle='-', linewidth=1, label='Detected Pitch')

        plt.title('Detected Pitches Over Time')
        plt.xlabel('Frame Index')
        plt.ylabel('Frequency (Hz)')
        plt.grid(True, which='both', linestyle='--', alpha=0.7)
        plt.legend()

        # Save or show
        print(f"Plotting {len(pitches)} data points...")
        plt.tight_layout()
        plt.show()

    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    # Adjust path if running from project root vs tests directory
    target_file = '../tests/testoutput/detected_pitches.txt'
    visualize_pitches(target_file)
