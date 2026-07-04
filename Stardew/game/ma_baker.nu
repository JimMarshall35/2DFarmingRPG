#!/usr/bin/env nu

def get_mandatory_item_property [
    item_name: string,
    property_name: string,
    attribute_name: string,
    content
] {
    (
        $content
        | where attributes.name  == $item_name
        | get content
        | first
        | where tag == $property_name
        | get attributes
        | get $attribute_name
        | first
    )
}

def get_item_bool_property [
    item_name: string,
    property_name: string
    content
] {
    get_mandatory_item_property  $item_name $property_name "bool" $content
}


def get_item_string_property [
    item_name: string,
    property_name: string
    content
] {
    get_mandatory_item_property  $item_name $property_name "str" $content
}

def get_item_c_function_name [
    item_name: string,
    function_name: string,
    content
] {
    let functions = (
        $content
        | where attributes.name  == $item_name
        | get content
        | first
        | where tag == $function_name
    )
    (if ( $functions | length ) > 0 {
        (
            $functions
            | get content
            | first
            | get attributes.name
            | first
        )
    }
    else {
        ""
    })
}

def generate_struct_field_line [
    field_name: string,
    item_name: string,
    function_name: string,
    content
] {
    let fn = get_item_c_function_name $item_name $function_name $content
    (
        if $fn != "" {
            $".($field_name) = ($fn),"
        }
        else {
            ""
        }
    )
}

def get_def_struct_name [
    item_name: string
] {
    let s: string = (
        $item_name
        | split row "-"
        | str capitalize
        | str join 
    )
    $"g($s)Def"
}

def get_c_item_def_strings [
    item_names: list<string>,
    content
] {
    let n = $item_names | each {|e|
        let function_lines = (
            [
                (generate_struct_field_line "onMakeCurrent" $e "on-make-current" $content)
                (generate_struct_field_line "onUseItem" $e "on-use-item" $content)
                (generate_struct_field_line "onMakeCurrent" $e "on-make-current" $content)
                (generate_struct_field_line "onTryEquip" $e "on-try-equip" $content)
                (generate_struct_field_line "onGameLayerPush" $e "on-gamelayer-push" $content)
                (generate_struct_field_line "onGameLayerPush" $e "on-gamelayer-pop" $content)
                (generate_struct_field_line "onStopBeingCurrent" $e "on-stop-being-current" $content)
            ] | where {|x| $x != ""}
        )
        let function_lines = $function_lines | each {|e1| $"    ($e1)"}

        $"
static struct WfItemDef (get_def_struct_name $e) = 
{
    .itemName         = \"($e)\",
    .UISpriteName     = \"(get_item_string_property $e "ui-sprite-name" $content)\",
    .pUserData        = NULL,
($function_lines | str join "\n")
    .onUseAnimation   = (get_item_string_property $e "on-use-animation" $content),
    .bCanUseItem      = (get_item_bool_property $e "can-use-item" $content),
    .pickupSpriteName = \"(get_item_string_property $e "pickup-sprite-name" $content)\",
};
"
    }
    $n
}

def get_add_item_c_fn_name [
    item_name: string
] {
    let s: string = (
        $item_name
        | split row "-"
        | str capitalize
        | str join 
    )
    $"WfAdd($s)Def"
}

def get_property_setter_lines [
    conf: list<any>,
    hashmap_name: string
] {
    (
        $conf 
        | each {|e| 
            match $e.tag {
                "Float" => { 
                    let val = $e.attributes.value
                    [
                        "prop.type = WfItemConfig_Float;"
                        $"prop.val.floatVal = ($val);"
                        $"HashmapInsert\(&($hashmap_name).properties, \"($e.attributes.name)\", &prop\);"
                    ]
                }
                "Int" => { 
                    let val = $e.attributes.value
                    [
                        "prop.type = WfItemConfig_Int;"
                        $"prop.val.intVal = ($val);"
                        $"HashmapInsert\(&($hashmap_name).properties, \"($e.attributes.name)\", &prop\);"
                    ]

                }
                "Bool" => { 
                    let val = $e.attributes.value
                    [
                        "prop.type = WfItemConfig_Bool;"
                        $"prop.val.boolVal = ($val);"
                        $"HashmapInsert\(&($hashmap_name).properties, \"($e.attributes.name)\", &prop\);"
                    ]
                }
                "String" => { 
                    let val = $e.attributes.value
                    [
                        "prop.type = WfItemConfig_String;"
                        $"prop.val.stringVal = \"($val)\";"
                        $"HashmapInsert\(&($hashmap_name).properties, \"($e.attributes.name)\", &prop\);"
                    ]

                }
                "Array" => { 
                    # TO BE IMPLEMENTED
                    []
                }
                _ => { [] }
            }
        }
        | flatten
    )
}

def get_config_init_lines [
    item_data, 
    item_name,
] {
    let conf = ($item_data | where tag == "config-data" | get content | first)

    (if ( $conf | length  ) > 0 {
        let def_name = get_def_struct_name $item_name
        
        $"
        struct WfItemConfigProperty prop;
        ($def_name).config.bSet = true;
        (get_property_setter_lines $conf $"($def_name).config" | str join "\n")
        "

    } else {
        ""
    })
}

def get_c_item_init_functions [
    item_names: list<string>,
    content
] {
    let n = $item_names | each {|e|
        let fn_name = get_add_item_c_fn_name $e
        let def_name = get_def_struct_name $e
        let item_data = $content
        | where attributes.name  == $e
        | get content
        | first

        {function_defintion: $"
static void ($fn_name)\(\)
{
    HashmapInit\(&($def_name).config.properties, 8, sizeof\(struct WfItemConfigProperty\)\);
    (get_config_init_lines $item_data $e)
    WfAddItemDef\(&($def_name)\);
}
", item_name: $e, function_call: $"($fn_name)\(\);" item_data: ($item_data | where tag == "config-data" | get content | first) }
    }
    $n
}

def assemble_c_file_string [
    item_def_structs,
    init_functions
] {
    $"
// Generated by Ma Baker
#include <stdlib.h>
#include \"WfItem.h\"
#include \"BakedItems.h\"
#include \"StringKeyHashMap.h\"
#include \"WfAllItems.h\"

( $item_def_structs | str join "\n\n" )

( $init_functions | get function_defintion | str join "\n\n" )

void WfInitBakedItems\(\)
{
($init_functions | get function_call | each {|e| '    ' + $e }| str join "\n")
}
    "
}

def main [
  items_xml_data_path: string,
  --out_c_folder: string, 
  --out_h_folder: string
] {
    "
        Bake the xml item defintions into c code.
    "
    let content = (
        cat $items_xml_data_path 
        | from xml 
        | get content
    )

    let item_names = $content | each {|e| $e.attributes.name }
    
    let item_def_structs = get_c_item_def_strings $item_names $content
    let init_functions = get_c_item_init_functions $item_names $content
    
    let c_file = assemble_c_file_string $item_def_structs $init_functions 
    $c_file | save ($out_c_folder | path join "BakedItems.c") -f

"
// Generated by Ma Baker
#ifndef WfBAKED_ITEMS_H
#define WFBAKED_ITEMS_H

void WfInitBakedItems();

#endif
" | save ($out_h_folder | path join "BakedItems.h") -f

    $c_file
}   
