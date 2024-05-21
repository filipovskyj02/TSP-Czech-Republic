import json

def parse_json_to_output_file(json_file, output_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        json_data = json.load(f)
        with open(output_file, 'w', encoding='utf-8') as out_f:
            for municipality in json_data['municipalities']:
                coordinates = municipality['souradnice']
                if (coordinates):
                    full_name = municipality['hezkyNazev']
                    out_f.write(f'"{full_name}" {coordinates[0]} {coordinates[1]}\n')

# Input JSON file path
json_file = "obce.json"

# Output file path
output_file = "output.txt"

# Parse JSON data from the input file and write to output file
parse_json_to_output_file(json_file, output_file)