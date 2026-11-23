## probably the shittest python code ever written
## a widget template generating tool.
## todo: refactor and make into a generic "find and replace" and "include text block conditionally" tool
## it is halfway there

## no functions, just one massive list of lines of code

import re

header_template_file_path = 'WidgetHeaderTemplate.txt'
srcFilePath = 'WidgetSrcTemplate.txt'

header_template_str = ""

src_template_str = ""

ifdef_regex = r'<<ifdef [a-zA-Z]*>>'

endifdef_regex = r'<<endif >>'

single_line_template_var_names_regex = r'<<[a-zA-Z]*>>'

multi_line_template_var_names_regex = r'<\[[a-zA-Z]*\]>'

with open(header_template_file_path, 'r') as file:
    header_template_str = file.read()

with open(srcFilePath, 'r') as file:
    src_template_str = file.read()


headerSingleLineNames = re.findall(single_line_template_var_names_regex, header_template_str)
headerMultiLineNames = re.findall(multi_line_template_var_names_regex, header_template_str)

srcSingleLineNames = re.findall(single_line_template_var_names_regex, src_template_str)
srcMultiLineNames = re.findall(multi_line_template_var_names_regex, src_template_str)

headerEndifs = re.findall(endifdef_regex, header_template_str);
headerIfdefs = re.findall(ifdef_regex, header_template_str);

srcEndifs = re.findall(endifdef_regex, src_template_str);
srcIfdefs = re.findall(ifdef_regex, src_template_str);


numHeaderIfdefs = len(headerIfdefs)
numHeaderEndifs = len(headerEndifs)
if numHeaderIfdefs != numHeaderEndifs:
    print(f"NUMBER OF HEADER IFDEFS {numHeaderIfdefs} DOES NOT EQUAL NUMBER OF ENDIFS {numHeaderEndifs}")
    input("lololololol")
    quit()

single_line_names_set = set()
multi_line_names_set = set()
ifdefs_set = set()

header_file_name = "unknown.h"
src_file_name = "unknown.c"


for val in headerSingleLineNames:
    single_line_names_set.add(val)
    
for val in srcSingleLineNames:
    single_line_names_set.add(val)

for val in headerMultiLineNames:
    multi_line_names_set.add(val)
    
for val in srcMultiLineNames:
    multi_line_names_set.add(val)

for val in headerIfdefs:
    ifdefs_set.add(val)

for val in srcIfdefs:
    ifdefs_set.add(val)

if not "<<WidgetName>>" in single_line_names_set:
    print("<<WidgetName>> is required")
    quit()


header_template_str_cpy = header_template_str
src_template_str_cpy = src_template_str

for val in single_line_names_set:       
    if val == "<<IncludeGuardDef>>":
        continue
    userVal = input(f"Enter value for {val}")
    if val == "<<WidgetName>>":
        header_file_name = f"{userVal}.h"
        src_file_name = f"{userVal}.c"
        include_guard_def_name = userVal.upper() + "_H"
        header_template_str_cpy = header_template_str_cpy.replace("<<IncludeGuardDef>>", include_guard_def_name)
        src_template_str_cpy = src_template_str_cpy.replace("<<IncludeGuardDef>>", include_guard_def_name)
        
    header_template_str_cpy = header_template_str_cpy.replace(val, userVal)
    src_template_str_cpy = src_template_str_cpy.replace(val, userVal)

for val in multi_line_names_set:
    bContinue = True
    replacementVal = ""
    while(bContinue):
        userInput = input(f"mutli line entry {val}. Enter a line and then a blank line to finish:")
        if userInput == "":
            break
        userInput += "\r\n"
        replacementVal += userInput
    header_template_str_cpy = header_template_str_cpy.replace(val, replacementVal)
    src_template_str_cpy = src_template_str_cpy.replace(val, replacementVal)

src_lines = src_template_str_cpy.splitlines()
header_lines = header_template_str_cpy.splitlines()

for ifdef in ifdefs_set:
    user_in = input(f"define {ifdef}? (y/n)");
    enable_ifdef = False
    while user_in != "y" and user_in != "n":
        user_in = input(f"define {ifdef}? (y/n)");
    if user_in == "y":
        enable_ifdef = True
    temp_lines = []
    is_skipping = False
    ifdef_count = 0
    for line in src_lines:
        if ifdef in line:
            is_skipping = not enable_ifdef
            ifdef_count += 1
        elif "<<endif >>" in line:
            if is_skipping:    
                is_skipping = False
                ifdef_count -= 1
            elif ifdef_count == 0:
                temp_lines.append(line)
            else:
                ifdef_count -= 1
        elif not is_skipping:
            temp_lines.append(line)
    src_template_str_cpy = "\n".join(temp_lines)
    print(src_template_str_cpy)
    src_lines = temp_lines
    temp_lines = []
    for line in header_lines:
        if ifdef in line:
            is_skipping = not enable_ifdef
            ifdef_count += 1
        elif "<<endif >>" in line:
            if is_skipping:    
                is_skipping = False
                ifdef_count -= 1
            elif ifdef_count == 0:
                temp_lines.append(line)
            else:
                ifdef_count -= 1
        elif not is_skipping:
            temp_lines.append(line)
    header_template_str_cpy = "\n".join(temp_lines)
    header_lines = temp_lines


with open(header_file_name, "w") as text_file:
    text_file.write(header_template_str_cpy)
    
with open(src_file_name, "w") as text_file:
    text_file.write(src_template_str_cpy)

















    
