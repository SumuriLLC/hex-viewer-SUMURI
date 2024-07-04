import re
import random

def generate_random_color():
    return "#{:06x}".format(random.randint(0, 0xFFFFFF))

def replace_color_codes(sql_file_path, output_file_path):
    with open(sql_file_path, 'r') as file:
        sql_content = file.read()

    # Regular expression to match color codes
    color_code_pattern = re.compile(r'#([0-9A-Fa-f]*)')

    # Replace all color codes with random color codes
    updated_sql_content = color_code_pattern.sub(lambda match: generate_random_color(), sql_content)

    # Write the updated content to a new file
    with open(output_file_path, 'w') as output_file:
        output_file.write(updated_sql_content)

    print(f"Updated SQL file saved as {output_file_path}")

# File paths
input_sql_file = 'tags_sql.sql' 
output_sql_file = 'tags_sql_processed.sql'  

# Replace color codes in the SQL file
replace_color_codes(input_sql_file, output_sql_file)
