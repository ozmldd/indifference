import argparse
import json
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate a C++ header from game assets.")
    parser.add_argument("--assets", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    arguments = parser.parse_args()

    assets_root = arguments.assets.resolve()
    if not assets_root.is_dir():
        parser.error(f"asset directory does not exist: {assets_root}")

    files = sorted(path for path in assets_root.rglob("*") if path.is_file())
    if not files:
        parser.error(f"no assets found in: {assets_root}")

    lines = [
        "#pragma once",
        "",
        "#include <cstddef>",
        "#include <string_view>",
        "",
        "struct EmbeddedAsset",
        "{",
        "    std::string_view path;",
        "    const unsigned char *data;",
        "    std::size_t size;",
        "};",
        "",
        "namespace",
        "{",
    ]

    for index, path in enumerate(files):
        data = path.read_bytes()
        lines.append(f"inline constexpr unsigned char embedded_asset_{index}[] = {{")
        for offset in range(0, len(data), 12):
            chunk = data[offset : offset + 12]
            lines.append("    " + ", ".join(f"0x{byte:02x}" for byte in chunk) + ",")
        lines.extend(["};", ""])

    lines.extend(["inline constexpr EmbeddedAsset embedded_assets[] = {"])
    for index, path in enumerate(files):
        relative_path = "assets/" + path.relative_to(assets_root).as_posix()
        lines.append(
            f"    {{{json.dumps(relative_path)}, embedded_asset_{index}, "
            f"sizeof(embedded_asset_{index})}},"
        )
    lines.extend(
        [
            "};",
            "}",
            "",
            "inline const EmbeddedAsset *findEmbeddedAsset(std::string_view path)",
            "{",
            "    for (const EmbeddedAsset &asset : embedded_assets)",
            "    {",
            "        if (asset.path == path)",
            "        {",
            "            return &asset;",
            "        }",
            "    }",
            "    return nullptr;",
            "}",
            "",
        ]
    )

    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    arguments.output.write_bytes("\n".join(lines).encode("utf-8"))


if __name__ == "__main__":
    main()
