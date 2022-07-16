from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="lib/Engine/Addresses.hpp", namespace="Engine::Addresses", groups=[
            Group(name="Engine", functions=[
                Item(name="Main",
                     pattern="40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?"),
            ]),
            Group(name="ResourceReference", functions=[
                Item(name="Load",
                     pattern="48 89 5C 24 10 57 48 83 EC 30 48 8D 59 08 48 8B F9 48 8B 0B 48 85 C9"),
                Item(name="Fetch",
                     pattern="40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 4B 08 48 85 C9 74 0A"),
                Item(name="Reset",
                     pattern="48 83 EC 38 45 33 C0 4C 89 01 48 8B 51 10 48 8B 41 08 48 89 54 24 28 4C 89 41 10 48 89 44 24 20"),
            ]),
        ]),
        Output(filename="lib/TweakDB/Addresses.hpp", namespace="TweakDB::Addresses", groups=[
            Group(functions=[
                Item(name="TweakDB_Load",
                     pattern="48 89 5C 24 18 55 57 41 56 48 8B EC 48 83 EC 70 48 8B D9 45 33 F6 48 8D"),
                Item(name="TweakDBID_Derive",
                     pattern="40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA"),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir())
