import argparse
import sys
import xml.etree.ElementTree as ET
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from pathlib import Path


def expand_templates(input_path: Path, output_path: Path) -> None:
    tree = ET.parse(input_path)
    root = tree.getroot()

    # Iterate over a copy since we're modifying the list in-place
    for i, child in enumerate(list(root)):
        if child.tag != "template":
            continue

        attrs = dict(child.attrib)

        template_path = attrs.pop("template_path", None)
        if template_path is None:
            print(
                f"Warning: <template> node at index {i} has no template_path attribute, skipping.",
                file=sys.stderr,
            )
            continue

        template_path = Path(template_path)
        if not template_path.is_absolute():
            # Resolve relative paths relative to the input file's directory
            template_path = input_path.parent / template_path

        if not template_path.exists():
            print(f"Error: template file not found: {template_path}", file=sys.stderr)
            sys.exit(1)

        # Render the Jinja template
        env = Environment(
            loader=FileSystemLoader(str(template_path.parent)),
            undefined=StrictUndefined,
        )
        jinja_template = env.get_template(template_path.name)
        rendered = jinja_template.render(**attrs)

        # Parse the rendered XML
        try:
            expanded_root = ET.fromstring(rendered)
        except ET.ParseError as e:
            print(
                f"Error: rendered template '{template_path}' is not valid XML: {e}",
                file=sys.stderr,
            )
            sys.exit(1)

        # Find the insertion index of this <template> node
        insert_index = list(root).index(child)

        # Remove the <template> node
        root.remove(child)

        # Insert the expanded children at the same position
        for offset, expanded_child in enumerate(expanded_root):
            root.insert(insert_index + offset, expanded_child)

    #ET.indent(tree, space="  ")
    tree.write(output_path, encoding="unicode", xml_declaration=True)
    print(f"Written to {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Expand <template> nodes in an XML file using Jinja2 templates."
    )
    parser.add_argument(
        "input",
        type=Path,
        help="Input XML file containing <template> nodes to expand.",
    )
    parser.add_argument(
        "--output",
        "-o",
        type=Path,
        required=True,
        help="Output XML file to write the expanded result to.",
    )
    args = parser.parse_args()

    if not args.input.exists():
        print(f"Error: input file not found: {args.input}", file=sys.stderr)
        sys.exit(1)

    expand_templates(args.input, args.output)


if __name__ == "__main__":
    main()