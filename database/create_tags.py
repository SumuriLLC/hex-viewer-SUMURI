import sqlite3
import os

# Database file name
database_file = 'tags_database.db'

# Delete the existing database file if it exists
if os.path.exists(database_file):
    os.remove(database_file)
    print(f"Deleted existing database file: {database_file}")

# Create a connection to the new SQLite database
conn = sqlite3.connect(database_file)
cursor = conn.cursor()

print("Created a new database and connected to it.")

# Create the "tags" table
cursor.execute('''
CREATE TABLE tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    offset INTEGER,
    length INTEGER,
    description TEXT,
    color TEXT,
    category TEXT,
    datatype TEXT
)
''')
print("Created the 'tags' table.")

# Function to insert data into the database
def insert_data(offset, length, description, color, category, datatype='number'):
    print(f"Inserting data: offset={offset}, length={length}, description={description}, color={color}, category={category}, datatype={datatype}")
    cursor.execute('''
    INSERT INTO tags (offset, length, description, color, category, datatype)
    VALUES (?, ?, ?, ?, ?, ?)
    ''', (offset, length, description, color, category, datatype))
    conn.commit()

# Function to process a text file and insert its contents into the database
def process_file(filepath, category):
    print(f"Processing file: {filepath} with category: {category}")
    with open(filepath, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            if len(parts) >= 4:
                start_offset_str, end_offset_str, description, color = parts[:4]
                datatype = parts[4] if len(parts) == 5 else 'number'
                start_offset = int(start_offset_str.split(' ')[0])
                end_offset = int(end_offset_str.split(' ')[0])
                length = end_offset - start_offset + 1
                insert_data(start_offset, length, description, color, category, datatype)
            else:
                print(f"Skipping invalid line: {line}")

# Directory containing the text files
directory = 'export'

print(f"Listing files in directory: {directory}")

# Filename to category mapping
filename_to_category = {
    'exfat_allocation_bitmap_directory_entry.txt': 'ExFAT Allocation Bitmap Directory Entry',
    'exfat_allocation_bitmap_table.txt': 'ExFAT Allocation Bitmap Table',
    'exfat_file_directory.txt': 'ExFAT File Directory Entry',
    'exfat_file_name_directory_entry.txt': 'ExFAT File Name Directory Entry',
    'exfat_strream_extension_dir.txt': 'ExFAT Stream Extension Directory Entry',
    'exfat_upcase.txt': 'ExFAT Upcase Table Directory Entry',
    'exfat_vbr.txt': 'exFAT VBR',
    'exfat_volume_label.txt': 'ExFAT Volume Label Directory Entry',
    'fat32vbr.txt': 'FAT32 VBR',
    'fat32_dos_alias.txt': 'FAT32 DOS Alias with LFN',
    'fat_32_allocation_table.txt': 'FAT32 File Allocation Table',
    'fat_32_sfn.txt': 'FAT32 SFN Directory Entry',
    'fat_vbr.txt': 'FAT VBR',
    'gpt_entry.txt': 'GPT Entry',
    'gpt_header.txt': 'GPT Header',
    'gpt_protective_mbr.txt': 'Protective MBR',
    'mbr.txt': 'MBR',
    'mft_data_attribute.txt': 'Data Attribute',
    'mft_filename_attribute.txt': 'Filename Attribute',
    'mft_file_record_header.txt': 'File Record Header',
    'mft_standard_attribute.txt': 'Standard Attribute',
    'mpt.txt': 'MPT',
    'ntfs_runlist_entry.txt': 'NTFS Runlist Entry',
    'ntfs_vbr.txt': 'NTFS VBR'
}

# Process each file in the directory
for filename in os.listdir(directory):
    filepath = os.path.join(directory, filename)
    print(f"Found file: {filename}")
    if filename in filename_to_category:
        category = filename_to_category[filename]
        process_file(filepath, category)
    else:
        print(f"Skipping file not in category mapping: {filename}")

# Close the database connection
conn.close()
print("Closed the database connection.")
