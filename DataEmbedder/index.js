const fs = require('fs');
const path = require('path');

const baseFolders = process.argv.slice(2).length ? process.argv.slice(2) : [ "Data", "scripts" ];
const functionFilePath = 'LeagueController/src/files_embedded.cpp';

// Read first line of functionFilePath to get the last time the file was updated
const dateString = fs.existsSync(functionFilePath) ? fs.readFileSync(functionFilePath, 'utf8').toString().replace("\r", "").split('\n')[0].substring(3) : 0;
const lastUpdate = new Date(dateString);

function writeIfDifferent(path, data) {
	if (!fs.existsSync(path) || fs.readFileSync(path).toString() != data) {
		fs.writeFileSync(path, data);
		console.log(`DataEmbedder: Updated ${path}`);
	}
	else {
		console.log(`DataEmbedder: No need to update ${path}`);
	}
}

function getAllFilesInFolders(folders) {
	let results = [];
	foldersToVisit = folders.slice();

	while (foldersToVisit.length) {
		const folder = foldersToVisit.splice(0, 1)[0];
		fs.readdirSync(folder).forEach(file => {
			file = folder + '/' + file;
			const stat = fs.statSync(file);

			if (stat && stat.isDirectory())
				foldersToVisit.push(file);
			else
				results.push(file);
		});
	}
	return results;
}

function checkIfHasNewerFiles(files) {
	const newestFile = files.reduce((newestFile, file) => {
		const stat = fs.statSync(file);
		return stat.mtime > newestFile.mtime ? stat : newestFile;
	}, { mtime: 0 });
	return newestFile.mtime > lastUpdate;
}

const files = getAllFilesInFolders(baseFolders)
	.filter(file => fs.statSync(file).isFile())
	.map(file => { return { file, data: fs.readFileSync(file) }});

function asHexString(byte) {
	return "0x" + ("0" + (byte & 0xFF).toString(16)).slice(-2);
}

function getSafeVariableName(string) {
	return "g_" + string.replace(/[^a-zA-Z0-9_]/g, "_");
}

if (checkIfHasNewerFiles(files.map(f => f.file)) == false) {
	console.log("DataEmbedder: No need to update files");
	return;
}
const code = files.reduce((code, file) => code + `\t\tEmbeddedFileSystem::Add("${file.file.toLowerCase()}", ${getSafeVariableName(file.file)}, ${Array.from(file.data).length});\n`, '');
const list = files.reduce((list, file) => list + `\tu8 ${getSafeVariableName(file.file)}[] = { ${Array.from(file.data).map(asHexString).join(', ')} };\n`, '');

functionFile = `// ${new Date().toISOString()}
#if NDEBUG
#include <league_controller/types.hpp>
#include <league_controller/profiler.hpp>
#include "filesystem_embedded.hpp"

// DO NOT OPEN THIS
#include "files_embedded_src.cpp"

namespace LeagueController
{
	void LoadEmbeddedData()
	{
		Leacon_Profiler;
${code}
	}
}
#endif`;
writeIfDifferent(functionFilePath, functionFile);

srcFile = `#if NDEBUG
#include <league_controller/types.hpp>

namespace LeagueController
{
${list}
}

#endif`;
writeIfDifferent('LeagueController/src/files_embedded_src.cpp', srcFile);
