from collections import Counter

def main():
    with open("test.txt") as f:
        lines = f.readlines()
        
    print(Counter(line.strip() for line in lines))
    

if __name__ == "__main__":
    main()
