-- Tables with descriptive text about the traits, to be displayed
-- during character creation process. - C. Blue

function get_trait_diz(r_title, l)
    local i
    i = 0
    while __traits_diz[i * 2 + 1] ~= nil do
        if __traits_diz[i * 2 + 1] == r_title then
            if __traits_diz[i * 2 + 2][l + 1] == nil then
                return ""
            else
                return __traits_diz[i * 2 + 2][l + 1]
            end
        end
        i = i + 1
    end
    return ""
end

__traits_diz = {

--  "12345678901234567890123456789012345678901234567890",

"N/A",
    {"", "", "", "", "", "", "", "", "", "", "", "",},


"Blue lineage",
    {"The \255ublue\255U lineage provides intrinsic resistance to",
    "lightning which turns into immunity later (25).",
    "It also brands your attacks with lightning (5)",
    "and surrounds you with a lightning aura (15).",
    "This lineage provides lightning breath.",
    "", "", "", "", "", "", "",},

"White lineage",
    {"The \255uwhite\255U lineage provides intrinsic resistance",
    "to frost which turns into immunity later (25).",
    "It also surrounds you with a freezing aura (15).",
    "On the downside, it is susceptible to fire.",
    "This lineage provides frost breath.",
    "", "", "", "", "", "", "",},

"Red lineage",
    {"The \255ured\255U lineage provides intrinsic resistance to",
    "fire, which turns into immunity later (25).",
    "On the downside, it is susceptible to cold.",
    "This lineage provides fire breath.",
    "", "", "", "", "", "", "", "",},

"Black lineage",
    {"The \255ublack\255U lineage provides intrinsic resistance",
    "to acid, which turns into immunity later (25).",
    "This lineage provides acid breath.",
    "", "", "", "", "", "", "", "", "",},

"Green lineage",
    {"The \255ugreen\255U lineage provides intrinsic resistance",
    "to poison, which turns into immunity later (25).",
    "This lineage provides poison breath.",
    "", "", "", "", "", "", "", "", "",},

"Multi-hued lineage",
    {"The \255umulti-hued\255U lineage provides intrinsic",
    "resistances to the four basic elements and poison (25).",
    "The basic resistances are obtained in the order of",
    "lightning (5), frost (10), fire (15) and acid (20).",
    "This lineage provides a random elemental breath.",
    "", "", "", "", "", "", "",},

"Bronze lineage",
    {"The \255ubronze\255U lineage provides intrinsic resistance",
    "to confusion (5), paralysis and electricity (10)",
    "and its metallic scales have a chance to",
    "reflect bolts and arrows (20).",
    "This lineage provides confusion breath.",
    "", "", "", "", "", "", "",},

"Silver lineage",
    {"The \255usilver\255U lineage provides intrinsic resistance",
    "to cold (5), acid (10) and poison (15)",
    "and its metallic scales have a chance to",
    "reflect bolts and arrows (20).",
    "This lineage provides inertia breath.",
    "", "", "", "", "", "", "",},

"Golden lineage",
    {"The \255ugolden\255U lineage provides intrinsic resistance",
    "to fire (5), acid (10) and sound (15)",
    "and its metallic scales have a chance to",
    "reflect bolts and arrows (20).",
    "This lineage provides sound breath.",
    "", "", "", "", "", "", "",},

"Law lineage",
    {"The \255ulaw\255U lineage provides intrinsic resistance to",
    "shards (5), paralysis (10) and sound (15).",
    "This lineage provides shards breath.",
    "", "", "", "", "", "", "", "", "",},

"Chaos lineage",
    {"The \255uchaos\255U lineage provides intrinsic resistance",
    "to confusion (5) which later turns into chaos (15)",
    "and also to disenchantment even later (20).",
    "This lineage provides chaos breath.",
    "", "", "", "", "", "", "", "",},

"Balance lineage",
    {"The \255ubalance\255U lineage provides intrinsic resistance",
    "to disenchantment (10) and sound (20).",
    "This lineage provides disenchantment breath.",
    "", "", "", "", "", "", "", "", "",},

"Power lineage",
    {"The \255upower\255U lineage provides intrinsic resistance",
    "to fear and blindness (5), and its metallic scales",
    "have a chance to reflect bolts and arrows (20).",
    "This lineage provides a random elemental breath.",
    "", "", "", "", "", "", "", "",},


"Enlightened",
    {"\255uEnlightenment\255U attunes to angelic powers.",
    "", "", "", "", "", "", "", "", "", "", "",},

"Corrupted",
    {"\255uCorruption\255U attunes to demonic powers.",
    "", "", "", "", "", "", "", "", "", "", "",},


"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},

"",
    {"\255u\255U",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",},
}
