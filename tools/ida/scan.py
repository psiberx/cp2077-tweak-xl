from cp77ida import Item, Group, Output, scan
from pathlib import Path


# Defines patterns and output files
def patterns():
    return [
        Output(filename="src/Red/Addresses.hpp", namespace="Red::Addresses", groups=[
            Group(functions=[
                Item(name="Main",
                     pattern="40 55 53 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8"),
            ]),
            Group(name="TweakDB", functions=[
                Item(name="Init",
                     pattern="48 89 5C 24 ? 57 48 83 EC ? 4C 8B 42 ? 48 8B D9 BF ? ? ? ? 4D 85 C0 74 ? 65 48 8B 04 25 ? ? ? ? 4D 8B 00 48 8B 10 41 8A 40 ? 88 04 17 48 8B 41 08 8B 08 85 C9 74 ? E8 ? ? ? ? E8"),
                Item(name="Load",
                     pattern="48 89 5C 24 ? 48 89 7C 24 ? 55 48 8B EC 48 ? EC ? ? ? ? 48 8B F9 48 8B DA 48 8B 0D"),
                Item(name="CreateRecord",
                     pattern="48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 8B C2 49 8B D8 83 E0 1F"),
            ]),
            Group(name="TweakDBID", functions=[
                Item(name="Derive",
                     pattern="48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 45 33 C9 48 8B FA"),
            ]),
            Group(name="StatsDataSystem", functions=[
                Item(name="InitializeRecords",
                     pattern="48 89 5C 24 ? 55 56 57 41 56 41 57 48 8B EC 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B C8 48 8B 10 4C 8B 42 ? 48 BA 8D 46 27 9F 31 66 30 74"),
                Item(name="InitializeParams",
                     pattern="40 55 53 56 57 41 56 48 8B EC 48 83 EC ? 8B 91 ? ? ? ? 48 8D B1 ? ? ? ? 48 8B F9 39 56 ? 73 ? 48 83 64 24"),
                Item(name="GetStatRange",
                     pattern="48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B FA 48 8B E9 41 81 F8 ? ? ? ? 73"),
                Item(name="GetStatFlags",
                     pattern="48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B FA 48 8B F1 81 FA ? ? ? ? 73"),
                Item(name="CheckStatFlag",
                     pattern="40 53 48 83 EC 20 41 8B D8 E8 ? ? ? ? 85 C3 0F 95 C0 48 83 C4 20 5B C3"),
            ]),
        ]),
    ]


# Defines base output dir
def output_dir():
    cwd = Path(__file__).resolve().parent
    return cwd.parent.parent  # 2 levels up


scan(patterns(), output_dir(), __file__)
