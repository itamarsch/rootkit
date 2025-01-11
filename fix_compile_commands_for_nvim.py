import json


def remove_invalid_options(file_path, invalid_options):
    """
    Remove invalid compiler options from a compile_commands.json file.

    :param file_path: Path to the JSON file.
    :param invalid_options: List of invalid options to remove.
    """
    try:
        with open(file_path, "r") as f:
            data = json.load(f)

        for entry in data:
            arguments = entry.get("arguments", [])
            entry["arguments"] = [
                arg
                for arg in arguments
                if not any(option in arg for option in invalid_options)
            ]

        with open(file_path, "w") as f:
            json.dump(data, f, indent=4)
        print(f"Invalid options removed and file saved to {file_path}")
    except Exception as e:
        print(f"Error processing file: {e}")


# List of invalid options to remove
invalid_options = [
    "-mpreferred-stack-boundary=3",
    "-fconserve-stack",
    "bounds-strict",
    "-fno-allow-store-data-races",
    "-mindirect-branch=thunk-extern",
    "-mindirect-branch-register",
    "-mrecord-mcount",
]

# Path to the JSON file
file_path = "compile_commands.json"

# Call the function
remove_invalid_options(file_path, invalid_options)
