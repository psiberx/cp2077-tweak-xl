from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="src/Red/Addresses.hpp", namespace="Red::Addresses", groups=[
            Group(functions=[
                Item(name="Main",
                     pattern="40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?"),
            ]),
            Group(name="TweakDB", functions=[
                Item(name="Load",
                     pattern="48 89 5C 24 18 55 57 41 56 48 8B EC 48 83 EC 70 48 8B D9 45 33 F6 48 8D"),
                Item(name="CreateRecord",
                     pattern="48 89 5C 24 08 ? 89 ? 24 18 57 48 83 EC 30 8B C2"),
            ]),
            Group(name="TweakDBID", functions=[
                Item(name="Derive",
                     pattern="40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA"),
            ]),
            Group(name="StatsDataSystem", functions=[
                Item(name="InitializeRecords",
                     pattern="48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 BA"),
                Item(name="InitializeParams",
                     pattern="48 8B C4 41 54 41 56 48 81 EC ? ? ? ? 8B 91 ? ? ? ? 4C 8D B1 ? ? ? ? 45 33 E4 48 89 58 ? 48 89 78"),
                Item(name="GetStatRange",
                     pattern="48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B FA 48 8B E9 41 81 F8 ? ? ? ? 73"),
                Item(name="GetStatFlags",
                     pattern="48 89 74 24 ? 57 48 83 EC ? 8B FA 48 8B F1 81 FA ? ? ? ? 73 ? 48 81 C1 ? ? ? ? 48 89 5C 24"),
                Item(name="CheckStatFlag",
                     pattern="48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B FA 41 8B F0 48 8B E9 81 FA ? ? ? ? 73 ? 48 81 C1"),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir(), __file__)
