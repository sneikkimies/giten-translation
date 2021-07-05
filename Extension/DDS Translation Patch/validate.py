
def validate(path):

    with open(path) as file:
        data = file.read()

    dataLines = data.split("\n")
    for n, line in enumerate(dataLines):
        if line.count("\t") != 1 and line.count("#") == 0 and len(line) > 2:
            print(n+1, line)
   
validate("Combat.txt")
validate("Translation.txt")
validate("Names.txt")
   
input()