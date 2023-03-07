const fs = require('fs');

let what = process.argv[2];

if (what == '1'){
	console.log(JSON.stringify({
		"name": "lb1l-vsc",
		"displayName": "lb1l-vsc",
		"description": "",
		"version": "0.0.1",
		"engines": {
			"vscode": "^1.76.0"
		},
		"categories": [
			"Programming Languages"
		],
		"contributes": {
			"languages": [{
				"id": "lb1l",
				"aliases": ["lb1l", "lb1l"],
				"extensions": [".lbl"],
				"configuration": "./language-configuration.json"
			}],
			"grammars": [{
				"language": "lb1l",
				"scopeName": "source.lb1l",
				"path": "./syntaxes/lb1l.tmLanguage.json"
			}]
		}
	}));
}else if (what == '2'){
	console.log(JSON.stringify({
    "comments": {
        // symbols used for start and end a block comment. Remove this entry if your language does not support block comments
        "blockComment": [ ";", ";" ]
    },
    // symbols that are auto closed when typing
    "autoClosingPairs": [
        [";", ";"],
        ["\"", "\""],
    ],
    // symbols that can be used to surround a selection
    "surroundingPairs": [
        [";", ";"],
        ["\"", "\""],
    ]
	}));
}else if (what == '3'){

	let CFUNS = Array.from(fs.readFileSync("lbll.h").toString().matchAll(/lbll_reg_cfun\( *"(.+?)"/g)).map(x=>x[1]);

	for (let i = 0; i < CFUNS.length; i++){
		let s = "";
		for (let j = 0; j < CFUNS[i].length; j++){
			if ("^|>".includes(CFUNS[i][j])){
				s += '\\';
			}
			s += CFUNS[i][j];
		}
		CFUNS[i] = s;
	}
	CFUNS = CFUNS.filter(x=>!x.includes('\\'))

	let tms = {
		"$schema": "https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json",
		"name": "lb1l",
		"patterns": [
			{
				"include": "#keywords"
			},
			{
				"include": "#strings"
			},
			{
				"include": "#characters"
			},
			{
				"include": "#constants"
			},
			{
				"include": "#comments"
			},
			{
				"include": "#types"
			},
			{
				"include": "#macros"
			},
			{
				"include": "#vars"
			}
		],
		"repository": {
			"comments": {
				"begin": "(?<!\\\\);",
				"beginCaptures": {
					"0": {
						"name": "punctuation.definition.comment.lb1l"
					}
				},
				"end": ";",
				"name": "comment.block.semicolon.lb1l"
			},
			"keywords": {
				"patterns": [{
					"name": "keyword.operator.lb1l",
					"match": "(@@|@|\-\>|\=\>|%%|%|\\^|\\~|\\>\\>\\||\\>\\>|\\^\\^|@@\\>|\\*)"
				},
				]
			},
			"types":{
				"patterns":[{
					"name": "entity.name.type.lb1l",
					"match": "\\b("+CFUNS.join('|')+")\\b"
				}]
			},
			"strings": {
				"name": "string.quoted.double.lb1l",
				"begin": "\"",
				"end": "\"",
				"patterns": [
					{
						"name": "constant.character.escape.lb1l",
						"match": "\\\\."
					}
				]
			},
			"constants": {
				"patterns": [
					{
						"match": "\\b(-?\\d+\\.\\d+([eE][+-]?\\d+)?)\\b",
						"name": "constant.numeric.float.lb1l"
					},
					{
						"match": "\\b(-?\\d+)\\b",
						"name": "constant.numeric.int.lb1l"
					}
				]
			}
		},
		"scopeName": "source.lb1l"
	}

	console.log(JSON.stringify(tms));
}