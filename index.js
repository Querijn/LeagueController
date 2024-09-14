const fs = require("fs");
const path = require("path");

// find all folders recursively with the name "src" and "inc"
function findInFolder(folder, name) {
	const folders = [folder];
	const found = [];
	while (folders.length) {
		const folder = folders.pop();
		const files = fs.readdirSync(folder);
		for (const file of files) {
			const fullPath = path.join(folder, file);
			if (file === name) {
				found.push(fullPath);
			}
			else if (file != "ext" && file != ".git" && fs.lstatSync(fullPath).isDirectory()) {
				folders.push(fullPath);
			}
		}
	}
	return found;
}

const folders = findInFolder("./", "src").concat( findInFolder("./", "inc") ).concat([ "scripts" ]);

// find all files with the extension ".hpp" or ".cpp"
function findFiles(folder, ext) {
	const files = [];
	const folders = [folder];
	while (folders.length) {
		const folder = folders.pop();
		const folderFiles = fs.readdirSync(folder);
		for (const file of folderFiles) {
			const fullPath = path.join(folder, file);
			if (file.endsWith(ext)) {
				files.push(fullPath);
			}
			else if (file != "ext" && file != ".git" && fs.lstatSync(fullPath).isDirectory()) {
				folders.push(fullPath);
			}
		}
	}
	return files;
}

let files = [];
for (const folder of folders) {
	files.push(...findFiles(folder, ".hpp"));
	files.push(...findFiles(folder, ".cpp"));
}

// Count lines of all files
let lines = 0;
for (const file of files) {
	const fileLines = fs.readFileSync(file, { encoding: "utf8" }).split("\n").length;
	lines += fileLines;
}
console.log("lines of CPP: " + lines);

files = [];
for (const folder of folders) {
	files.push(...findFiles(folder, ".lua"));
}

// Count lines of all files
lines = 0;
for (const file of files) {
	const fileLines = fs.readFileSync(file, { encoding: "utf8" }).split("\n").length;
	lines += fileLines;
}
console.log("lines of Lua: " + lines);
debugger;