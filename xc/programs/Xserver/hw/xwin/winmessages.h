static const unsigned MESSAGE_NAMES_LEN =1024;
static const char *MESSAGE_NAMES[1024] = {
	"WM_NULL",
	"WM_CREATE",
	"WM_DESTROY",
	"WM_MOVE",
	"4",
	"WM_SIZE",
	"WM_ACTIVATE",
	"WM_SETFOCUS",
	"WM_KILLFOCUS",
	"9",
	"WM_ENABLE",
	"WM_SETREDRAW",
	"WM_SETTEXT",
	"WM_GETTEXT",
	"WM_GETTEXTLENGTH",
	"WM_PAINT",
	"WM_CLOSE",
	"WM_QUERYENDSESSION",
	"WM_QUIT",
	"WM_QUERYOPEN",
	"WM_ERASEBKGND",
	"WM_SYSCOLORCHANGE",
	"WM_ENDSESSION",
	"23",
	"WM_SHOWWINDOW",
	"25",
	"WM_WININICHANGE",
	"WM_DEVMODECHANGE",
	"WM_ACTIVATEAPP",
	"WM_FONTCHANGE",
	"WM_TIMECHANGE",
	"WM_CANCELMODE",
	NULL /* WM_SETCURSOR */,
	"WM_MOUSEACTIVATE",
	"WM_CHILDACTIVATE",
	"WM_QUEUESYNC",
	"WM_GETMINMAXINFO",
	"37",
	"WM_PAINTICON",
	"WM_ICONERASEBKGND",
	"WM_NEXTDLGCTL",
	"41",
	"WM_SPOOLERSTATUS",
	"WM_DRAWITEM",
	"WM_MEASUREITEM",
	"WM_DELETEITEM",
	"WM_VKEYTOITEM",
	"WM_CHARTOITEM",
	"WM_SETFONT",
	"WM_GETFONT",
	"WM_SETHOTKEY",
	"WM_GETHOTKEY",
	"52",
	"53",
	"54",
	"WM_QUERYDRAGICON",
	"56",
	"WM_COMPAREITEM",
	"58",
	"59",
	"60",
	"61",
	"62",
	"63",
	"64",
	"WM_COMPACTING",
	"66",
	"67",
	"WM_COMMNOTIFY",
	"69",
	"WM_WINDOWPOSCHANGING",
	"WM_WINDOWPOSCHANGED",
	"WM_POWER",
	"73",
	"WM_COPYDATA",
	"WM_CANCELJOURNAL",
	"76",
	"77",
	"WM_NOTIFY",
	"79",
	"WM_INPUTLANGCHANGEREQUEST",
	"WM_INPUTLANGCHANGE",
	"WM_TCARD",
	"WM_HELP",
	"WM_USERCHANGED",
	"WM_NOTIFYFORMAT",
	"86",
	"87",
	"88",
	"89",
	"90",
	"91",
	"92",
	"93",
	"94",
	"95",
	"96",
	"97",
	"98",
	"99",
	"100",
	"101",
	"102",
	"103",
	"104",
	"105",
	"106",
	"107",
	"108",
	"109",
	"110",
	"111",
	"112",
	"113",
	"114",
	"115",
	"116",
	"117",
	"118",
	"119",
	"120",
	"121",
	"122",
	"WM_CONTEXTMENU",
	"WM_STYLECHANGING",
	"WM_STYLECHANGED",
	"WM_DISPLAYCHANGE",
	"WM_GETICON",
	"WM_SETICON",
	"WM_NCCREATE",
	"WM_NCDESTROY",
	"WM_NCCALCSIZE",
	NULL /* WM_NCHITTEST */,
	"WM_NCPAINT",
	"WM_NCACTIVATE",
	"WM_GETDLGCODE",
	"WM_SYNCPAINT",
	"137",
	"138",
	"139",
	"140",
	"141",
	"142",
	"143",
	"144",
	"145",
	"146",
	"147",
	"148",
	"149",
	"150",
	"151",
	"152",
	"153",
	"154",
	"155",
	"156",
	"157",
	"158",
	"159",
	NULL /* WM_NCMOUSEMOVE */,
	"WM_NCLBUTTONDOWN",
	"WM_NCLBUTTONUP",
	"WM_NCLBUTTONDBLCLK",
	"WM_NCRBUTTONDOWN",
	"WM_NCRBUTTONUP",
	"WM_NCRBUTTONDBLCLK",
	"WM_NCMBUTTONDOWN",
	"WM_NCMBUTTONUP",
	"WM_NCMBUTTONDBLCLK",
	"170",
	"171",
	"172",
	"173",
	"174",
	"175",
	"176",
	"177",
	"178",
	"179",
	"180",
	"181",
	"182",
	"183",
	"184",
	"185",
	"186",
	"187",
	"188",
	"189",
	"190",
	"191",
	"192",
	"193",
	"194",
	"195",
	"196",
	"197",
	"198",
	"199",
	"200",
	"201",
	"202",
	"203",
	"204",
	"205",
	"206",
	"207",
	"208",
	"209",
	"210",
	"211",
	"212",
	"213",
	"214",
	"215",
	"216",
	"217",
	"218",
	"219",
	"220",
	"221",
	"222",
	"223",
	"224",
	"225",
	"226",
	"227",
	"228",
	"229",
	"230",
	"231",
	"232",
	"233",
	"234",
	"235",
	"236",
	"237",
	"238",
	"239",
	"240",
	"241",
	"242",
	"243",
	"244",
	"245",
	"246",
	"247",
	"248",
	"249",
	"250",
	"251",
	"252",
	"253",
	"254",
	"255",
	"WM_KEYDOWN",
	"WM_KEYUP",
	"WM_CHAR",
	"WM_DEADCHAR",
	"WM_SYSKEYDOWN",
	"WM_SYSKEYUP",
	"WM_SYSCHAR",
	"WM_SYSDEADCHAR",
	"WM_CONVERTREQUESTEX",
	"265",
	"266",
	"267",
	"268",
	"WM_IME_STARTCOMPOSITION",
	"WM_IME_ENDCOMPOSITION",
	"WM_IME_KEYLAST",
	"WM_INITDIALOG",
	"WM_COMMAND",
	"WM_SYSCOMMAND",
	NULL /* WM_TIMER */,
	"WM_HSCROLL",
	"WM_VSCROLL",
	"WM_INITMENU",
	"WM_INITMENUPOPUP",
	"280",
	"281",
	"282",
	"283",
	"284",
	"285",
	"286",
	"WM_MENUSELECT",
	"WM_MENUCHAR",
	"WM_ENTERIDLE",
	"290",
	"291",
	"292",
	"293",
	"294",
	"295",
	"296",
	"297",
	"298",
	"299",
	"300",
	"301",
	"302",
	"303",
	"304",
	"305",
	"WM_CTLCOLORMSGBOX",
	"WM_CTLCOLOREDIT",
	"WM_CTLCOLORLISTBOX",
	"WM_CTLCOLORBTN",
	"WM_CTLCOLORDLG",
	"WM_CTLCOLORSCROLLBAR",
	"WM_CTLCOLORSTATIC",
	"313",
	"314",
	"315",
	"316",
	"317",
	"318",
	"319",
	"320",
	"321",
	"322",
	"323",
	"324",
	"325",
	"326",
	"327",
	"328",
	"329",
	"330",
	"331",
	"332",
	"333",
	"334",
	"335",
	"336",
	"337",
	"338",
	"339",
	"340",
	"341",
	"342",
	"343",
	"344",
	"345",
	"346",
	"347",
	"348",
	"349",
	"350",
	"351",
	"352",
	"353",
	"354",
	"355",
	"356",
	"357",
	"358",
	"359",
	"360",
	"361",
	"362",
	"363",
	"364",
	"365",
	"366",
	"367",
	"368",
	"369",
	"370",
	"371",
	"372",
	"373",
	"374",
	"375",
	"376",
	"377",
	"378",
	"379",
	"380",
	"381",
	"382",
	"383",
	"384",
	"385",
	"386",
	"387",
	"388",
	"389",
	"390",
	"391",
	"392",
	"393",
	"394",
	"395",
	"396",
	"397",
	"398",
	"399",
	"400",
	"401",
	"402",
	"403",
	"404",
	"405",
	"406",
	"407",
	"408",
	"409",
	"410",
	"411",
	"412",
	"413",
	"414",
	"415",
	"416",
	"417",
	"418",
	"419",
	"420",
	"421",
	"422",
	"423",
	"424",
	"425",
	"426",
	"427",
	"428",
	"429",
	"430",
	"431",
	"432",
	"433",
	"434",
	"435",
	"436",
	"437",
	"438",
	"439",
	"440",
	"441",
	"442",
	"443",
	"444",
	"445",
	"446",
	"447",
	"448",
	"449",
	"450",
	"451",
	"452",
	"453",
	"454",
	"455",
	"456",
	"457",
	"458",
	"459",
	"460",
	"461",
	"462",
	"463",
	"464",
	"465",
	"466",
	"467",
	"468",
	"469",
	"470",
	"471",
	"472",
	"473",
	"474",
	"475",
	"476",
	"477",
	"478",
	"479",
	"480",
	"481",
	"482",
	"483",
	"484",
	"485",
	"486",
	"487",
	"488",
	"489",
	"490",
	"491",
	"492",
	"493",
	"494",
	"495",
	"496",
	"497",
	"498",
	"499",
	"500",
	"501",
	"502",
	"503",
	"504",
	"505",
	"506",
	"507",
	"508",
	"509",
	"510",
	"511",
	NULL /* WM_MOUSEMOVE */,
	"WM_LBUTTONDOWN",
	"WM_LBUTTONUP",
	"WM_LBUTTONDBLCLK",
	"WM_RBUTTONDOWN",
	"WM_RBUTTONUP",
	"WM_RBUTTONDBLCLK",
	"WM_MBUTTONDOWN",
	"WM_MBUTTONUP",
	"WM_MBUTTONDBLCLK",
	"WM_MOUSEWHEEL",
	"523",
	"524",
	"525",
	"526",
	"527",
	"WM_PARENTNOTIFY",
	"WM_ENTERMENULOOP",
	"WM_EXITMENULOOP",
	"WM_NEXTMENU",
	"WM_SIZING",
	"WM_CAPTURECHANGED",
	"WM_MOVING",
	"535",
	"WM_POWERBROADCAST",
	"WM_DEVICECHANGE",
	"538",
	"539",
	"540",
	"541",
	"542",
	"543",
	"WM_MDICREATE",
	"WM_MDIDESTROY",
	"WM_MDIACTIVATE",
	"WM_MDIRESTORE",
	"WM_MDINEXT",
	"WM_MDIMAXIMIZE",
	"WM_MDITILE",
	"WM_MDICASCADE",
	"WM_MDIICONARRANGE",
	"WM_MDIGETACTIVE",
	"554",
	"555",
	"556",
	"557",
	"558",
	"559",
	"WM_MDISETMENU",
	"WM_ENTERSIZEMOVE",
	"WM_EXITSIZEMOVE",
	"WM_DROPFILES",
	"WM_MDIREFRESHMENU",
	"565",
	"566",
	"567",
	"568",
	"569",
	"570",
	"571",
	"572",
	"573",
	"574",
	"575",
	"576",
	"577",
	"578",
	"579",
	"580",
	"581",
	"582",
	"583",
	"584",
	"585",
	"586",
	"587",
	"588",
	"589",
	"590",
	"591",
	"592",
	"593",
	"594",
	"595",
	"596",
	"597",
	"598",
	"599",
	"600",
	"601",
	"602",
	"603",
	"604",
	"605",
	"606",
	"607",
	"608",
	"609",
	"610",
	"611",
	"612",
	"613",
	"614",
	"615",
	"616",
	"617",
	"618",
	"619",
	"620",
	"621",
	"622",
	"623",
	"624",
	"625",
	"626",
	"627",
	"628",
	"629",
	"630",
	"631",
	"632",
	"633",
	"634",
	"635",
	"636",
	"637",
	"638",
	"639",
	"640",
	"WM_IME_SETCONTEXT",
	"WM_IME_NOTIFY",
	"WM_IME_CONTROL",
	"WM_IME_COMPOSITIONFULL",
	"WM_IME_SELECT",
	"WM_IME_CHAR",
	"647",
	"648",
	"649",
	"650",
	"651",
	"652",
	"653",
	"654",
	"655",
	"WM_IME_KEYDOWN",
	"WM_IME_KEYUP",
	"658",
	"659",
	"660",
	"661",
	"662",
	"663",
	"664",
	"665",
	"666",
	"667",
	"668",
	"669",
	"670",
	"671",
	"672",
	"WM_MOUSEHOVER",
	"674",
	"WM_MOUSELEAVE",
	"676",
	"677",
	"678",
	"679",
	"680",
	"681",
	"682",
	"683",
	"684",
	"685",
	"686",
	"687",
	"688",
	"689",
	"690",
	"691",
	"692",
	"693",
	"694",
	"695",
	"696",
	"697",
	"698",
	"699",
	"700",
	"701",
	"702",
	"703",
	"704",
	"705",
	"706",
	"707",
	"708",
	"709",
	"710",
	"711",
	"712",
	"713",
	"714",
	"715",
	"716",
	"717",
	"718",
	"719",
	"720",
	"721",
	"722",
	"723",
	"724",
	"725",
	"726",
	"727",
	"728",
	"729",
	"730",
	"731",
	"732",
	"733",
	"734",
	"735",
	"736",
	"737",
	"738",
	"739",
	"740",
	"741",
	"742",
	"743",
	"744",
	"745",
	"746",
	"747",
	"748",
	"749",
	"750",
	"751",
	"752",
	"753",
	"754",
	"755",
	"756",
	"757",
	"758",
	"759",
	"760",
	"761",
	"762",
	"763",
	"764",
	"765",
	"766",
	"767",
	"WM_CUT",
	"WM_COPY",
	"WM_PASTE",
	"WM_CLEAR",
	"WM_UNDO",
	"WM_RENDERFORMAT",
	"WM_RENDERALLFORMATS",
	"WM_DESTROYCLIPBOARD",
	"WM_DRAWCLIPBOARD",
	"WM_PAINTCLIPBOARD",
	"WM_VSCROLLCLIPBOARD",
	"WM_SIZECLIPBOARD",
	"WM_ASKCBFORMATNAME",
	"WM_CHANGECBCHAIN",
	"WM_HSCROLLCLIPBOARD",
	"WM_QUERYNEWPALETTE",
	"WM_PALETTEISCHANGING",
	"WM_PALETTECHANGED",
	"WM_HOTKEY",
	"787",
	"788",
	"789",
	"790",
	"WM_PRINT",
	"WM_PRINTCLIENT",
	"793",
	"794",
	"795",
	"796",
	"797",
	"798",
	"799",
	"800",
	"801",
	"802",
	"803",
	"804",
	"805",
	"806",
	"807",
	"808",
	"809",
	"810",
	"811",
	"812",
	"813",
	"814",
	"815",
	"816",
	"817",
	"818",
	"819",
	"820",
	"821",
	"822",
	"823",
	"824",
	"825",
	"826",
	"827",
	"828",
	"829",
	"830",
	"831",
	"832",
	"833",
	"834",
	"835",
	"836",
	"837",
	"838",
	"839",
	"840",
	"841",
	"842",
	"843",
	"844",
	"845",
	"846",
	"847",
	"848",
	"849",
	"850",
	"851",
	"852",
	"853",
	"854",
	"855",
	"856",
	"857",
	"858",
	"859",
	"860",
	"861",
	"862",
	"863",
	"864",
	"865",
	"866",
	"867",
	"868",
	"869",
	"870",
	"871",
	"872",
	"873",
	"874",
	"875",
	"876",
	"877",
	"878",
	"879",
	"880",
	"881",
	"882",
	"883",
	"884",
	"885",
	"886",
	"887",
	"888",
	"889",
	"890",
	"891",
	"892",
	"893",
	"894",
	"895",
	"896",
	"897",
	"898",
	"899",
	"900",
	"901",
	"902",
	"903",
	"904",
	"905",
	"906",
	"907",
	"908",
	"909",
	"910",
	"911",
	"912",
	"913",
	"914",
	"915",
	"916",
	"917",
	"918",
	"919",
	"920",
	"921",
	"922",
	"923",
	"924",
	"925",
	"926",
	"927",
	"928",
	"929",
	"930",
	"931",
	"932",
	"933",
	"934",
	"935",
	"936",
	"937",
	"938",
	"939",
	"940",
	"941",
	"942",
	"943",
	"944",
	"945",
	"946",
	"947",
	"948",
	"949",
	"950",
	"951",
	"952",
	"953",
	"954",
	"955",
	"956",
	"957",
	"958",
	"959",
	"960",
	"961",
	"962",
	"963",
	"964",
	"965",
	"966",
	"967",
	"968",
	"969",
	"970",
	"971",
	"972",
	"973",
	"974",
	"975",
	"976",
	"977",
	"978",
	"979",
	"980",
	"981",
	"982",
	"983",
	"984",
	"985",
	"986",
	"987",
	"988",
	"989",
	"990",
	"991",
	"992",
	"993",
	"994",
	"995",
	"996",
	"997",
	"998",
	"999",
	"1000",
	"1001",
	"1002",
	"1003",
	"1004",
	"1005",
	"1006",
	"1007",
	"1008",
	"1009",
	"1010",
	"1011",
	"1012",
	"1013",
	"1014",
	"1015",
	"1016",
	"1017",
	"1018",
	"1019",
	"1020",
	"1021",
	"1022",
	"1023"
};
