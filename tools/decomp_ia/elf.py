"""Lecture minimale des fichiers ELF32 little-endian produits par dsd et mwccarm."""

import struct
from dataclasses import dataclass, field


@dataclass
class Symbole:
    nom: str
    valeur: int
    taille: int
    type: int  # 0 notype, 1 object, 2 func, 3 section
    lien: int  # 0 local, 1 global, 2 weak
    section: int


@dataclass
class Section:
    nom: str
    type: int
    adresse: int
    offset: int
    taille: int
    lien: int
    info: int
    donnees: bytes = b""


@dataclass
class Reloc:
    offset: int
    type: int
    symbole: Symbole


@dataclass
class Elf:
    chemin: str
    sections: list[Section] = field(default_factory=list)
    symboles: list[Symbole] = field(default_factory=list)
    # index de section -> relocations qui s'y appliquent
    relocs: dict[int, list[Reloc]] = field(default_factory=dict)

    def section(self, nom: str) -> tuple[int, Section] | None:
        for i, s in enumerate(self.sections):
            if s.nom == nom:
                return i, s
        return None

    def symbole(self, nom: str) -> Symbole | None:
        for s in self.symboles:
            if s.nom == nom and s.section != 0:
                return s
        return None

    def symboles_definis(self) -> list[Symbole]:
        return [s for s in self.symboles if s.section not in (0, 0xFFF1) and s.type in (0, 1, 2) and s.nom
                and not s.nom.startswith("$")]


def lire(chemin: str, avec_donnees: bool = True) -> Elf:
    with open(chemin, "rb") as f:
        b = f.read()
    if b[:4] != b"\x7fELF":
        raise ValueError(f"{chemin} n'est pas un ELF")
    (e_shoff,) = struct.unpack_from("<I", b, 0x20)
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from("<HHH", b, 0x2E)
    elf = Elf(chemin)
    brutes = []
    for i in range(e_shnum):
        nom, typ, _flags, adr, off, taille, lien, info, _al, _ent = struct.unpack_from(
            "<IIIIIIIIII", b, e_shoff + i * e_shentsize)
        brutes.append((nom, typ, adr, off, taille, lien, info))
    shstr = brutes[e_shstrndx]
    def chaine(table_off, idx):
        fin = b.index(b"\0", table_off + idx)
        return b[table_off + idx:fin].decode("latin-1")
    for nom, typ, adr, off, taille, lien, info in brutes:
        s = Section(chaine(shstr[3], nom), typ, adr, off, taille, lien, info)
        if avec_donnees and typ != 8:  # SHT_NOBITS
            s.donnees = b[off:off + taille]
        elf.sections.append(s)
    for i, s in enumerate(elf.sections):
        if s.type == 2:  # SHT_SYMTAB
            strtab = elf.sections[s.lien]
            for k in range(s.taille // 16):
                st_nom, st_val, st_taille, st_info, _o, st_shndx = struct.unpack_from(
                    "<IIIBBH", b, s.offset + k * 16)
                elf.symboles.append(Symbole(chaine(strtab.offset, st_nom), st_val, st_taille,
                                            st_info & 0xF, st_info >> 4, st_shndx))
    if avec_donnees:
        for s in elf.sections:
            if s.type == 9:  # SHT_REL
                liste = elf.relocs.setdefault(s.info, [])
                for k in range(s.taille // 8):
                    r_off, r_info = struct.unpack_from("<II", b, s.offset + k * 8)
                    liste.append(Reloc(r_off, r_info & 0xFF, elf.symboles[r_info >> 8]))
    return elf
