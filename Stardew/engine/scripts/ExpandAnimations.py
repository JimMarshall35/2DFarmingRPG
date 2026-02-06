import argparse
import xml.etree.ElementTree as ET


def do_cmd_args():
    parser = argparse.ArgumentParser(
                    prog='ConvertTiled',
                    description="""
                    Expands </animation> nodes in xml files.
                    You can write an animation node like this in an atlas xml file:

                    <animation name="walk-base-female-down"
                                fps="10.0"
                                source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png"
                                startx="0"
                                starty="0"
                                incx="64"
                                incy="64"
                                width="64"
                                height="64"
                                numFrames="9"/>
                    And this script will expand it like so:

                    <animation-frames name="walk-base-female-down" fps="10.0">
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="0" width="64" height="64" name="walk-base-female-down0" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="64" width="64" height="64" name="walk-base-female-down1" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="128" width="64" height="64" name="walk-base-female-down2" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="192" width="64" height="64" name="walk-base-female-down3" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="256" width="64" height="64" name="walk-base-female-down4" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="320" width="64" height="64" name="walk-base-female-down5" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="384" width="64" height="64" name="walk-base-female-down6" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="448" width="64" height="64" name="walk-base-female-down7" />
                        <sprite source="./WfAssets/Image/lpc_base_assets/LPC Base Assets/sprites/people/female_walkcycle.png" top="128" left="512" width="64" height="64" name="walk-base-female-down8" />
                    </animation-frames>
                    """,
                    epilog='Jim Marshall - 2025')
    parser.add_argument("inFile", type=str, help="The XML atlas file to expand")
    parser.add_argument("-o", "--output_path", default="./expanded.xml")
    args = parser.parse_args()
    return args

def expand_animation(animation_node):
    animation_node.tag = "animation-frames"    # change tag
    animation_name = animation_node.attrib["name"]
    source = animation_node.attrib["source"]
    startx = int(animation_node.attrib["startx"])
    starty = int(animation_node.attrib["starty"])
    incx = int(animation_node.attrib["incx"])
    incy = int(animation_node.attrib["incy"])
    numFrames = int(animation_node.attrib["numFrames"])
    fps = animation_node.attrib["fps"]
    width = int(animation_node.attrib["width"])
    height = int(animation_node.attrib["height"])
    bMinimizeSpace = bool(animation_node.attrib["bMinimizeSpace"])
    bReverse = bool(animation_node.attrib["bReverse"]) if "bReverse" in animation_node.attrib else False


    bPalindrome = bool(animation_node.attrib["bPalindrome"]) if "bPalindrome" in animation_node.attrib else False

    animation_node.attrib = {
        "name" : animation_name,
        "fps" : fps
    }
    frames = []
    for i in range(numFrames):
        frames.append({
            "source" : source,
            "top" : str(starty),
            "left" : str(startx),
            "width" : str(width),
            "height" : str(height),
            "bMinimizeSpace" : "true" if bMinimizeSpace else "false",
            "name" : f"{animation_name}{i}"
        })
        startx += incx
        starty += incy
    if bReverse:
        frames = list(reversed(frames))
    if bPalindrome:
        for i in range(len(frames) - 2, -1, -1):
            frames.append(frames[i])
    for frame in frames:
        ET.SubElement(animation_node, "sprite", frame)


def main():
    args = do_cmd_args()
    tree = ET.parse(args.inFile)
    root = tree.getroot()
    animations = root.findall("animation")
    for anim in animations:
        expand_animation(anim)
    ET.indent(tree, space="\t", level=0)
    tree.write(args.output_path)
main()