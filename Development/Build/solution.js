var fs = require('fs');
var path = require('path');

function ProcessPropertyFile (proj, filename) {
	var path = proj.path + filename;
	path = path.replace(/\\/g, "/");
	var lines = fs.readFileSync(path, 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos;
		pos = lines[i].indexOf('<AdditionalIncludeDirectories');
		if(pos>0) {
			pos = lines[i].indexOf('>');
			var value = lines[i].substring(pos+1);
			value = value.substring(0, value.indexOf('<'));
			value = value.replace(/\\/g, "/");
			value = value.replace(/\;\%\(AdditionalIncludeDirectories\)/g, "/");
			value = value.split(';');
			for(i=0; i<value.length; i++) {
				if(value[i].indexOf("$(SolutionDir)")>=0) {
					value[i] = value[i].replace(/\$\(SolutionDir\)/g, "");
				} else {
					value[i] = proj.path + value[i];
				}
			}
			return value;
		}
	}
	return [];
}

function ProjectFile () {
	this.path = '';
	this.name = '';
	this.deps = [];
	this.type = '';
	this.inc_files = [];
	this.ddl_files = [];
	this.rpc_files = [];
	this.src_files = [];
	this.inc_dir = [];
	this.ext_deps = [];
}

ProjectFile.prototype.load = function (filename) {
	this.path = filename.substring(0, filename.lastIndexOf('\\')+1);
	this.name = filename.substring(filename.lastIndexOf('\\')+1);
	this.name = this.name.substring(0, this.name.lastIndexOf('.'));

	filename = filename.replace(/\\/g, "/");

	var lines = fs.readFileSync(path.normalize(filename), 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos;

		pos = lines[i].indexOf('<Import Project="');
		if(pos>0) {
			pos = lines[i].indexOf('"');
			var value = lines[i].substring(pos+1);
			value = value.substring(0, value.indexOf('"'));
			if(value.indexOf('$')>=0) {
				continue;
			}
			pos = Math.max(value.lastIndexOf('/'), value.lastIndexOf('\\'));
			switch(value.substring(pos + 1)) {
			case 'Win32.props':
			    break;
			case 'wxWidgets.props':
				this.ext_deps.push("wxWidgets");
			    break;
			case 'mysql.props':
				this.ext_deps.push("mysql");
			    break;
            default:
                this.inc_dir = this.inc_dir.concat(ProcessPropertyFile(this, value));
            }
			continue;
		}

		pos = lines[i].indexOf('<ConfigurationType>');
		if(pos>0) {
			pos = lines[i].indexOf('>');
			var name = lines[i].substring(pos+1);
			name = name.substring(0, name.indexOf('<'));
			this.type = name;
			continue;
		}

		pos = lines[i].indexOf('<ProjectReference Include="');
		if(pos>0) {
			var name = lines[i];
			name = name.substring(name.lastIndexOf('\\')+1);
			name = name.substring(0, name.indexOf('.vcxproj'));
			this.deps.push(name);
			continue;
		}

		var match_list = [
			'<ClCompile Include="',
			'ClInclude Include="',
			'<None Include="',
			'<CustomBuild Include="'
		];

		for(var t=0; t<match_list.length; t++) {
			var pos = lines[i].indexOf(match_list[t]);
			if(pos<0) continue;
			var filename = lines[i].substring(pos + match_list[t].length);
			pos = filename.indexOf('"');
			if(pos<0) continue;
			filename = filename.substring(0, pos);
			switch(path.extname(filename)) {
			case '.ddl':
				this.ddl_files.push(filename);
				break;
			case '.rpc':
				this.rpc_files.push(filename);
				break;
			case '.h':
			case '.hpp':
				this.inc_files.push(filename);
			case '.c':
			case '.cc':
			case '.cpp':
			case '.m':
			case '.mm':
				this.src_files.push(filename);
				break;
			}
		}
	}
};

function SolutionFile () {
	this.projs = [];
}

SolutionFile.prototype.load = function (filename) {
	var lines = fs.readFileSync(filename, 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos = lines[i].indexOf('Project("');
		if(pos<0) continue;
		pos = lines[i].indexOf(',');
		if(pos<0) throw "invalid solution file";
		var name = lines[i].substring(pos+3);
		pos = name.indexOf('"');
		if(pos<0) throw "invalid solution file";
		name = name.substring(0, pos);
		pos = name.indexOf('.vcxproj');
		if(pos<0) continue;
		this.projs.push(name);
	}

	for(var i=0; i<this.projs.length; i++) {
		var proj = new ProjectFile();
		proj.load(this.projs[i]);
		this.projs[i] = proj;
	}

	return true;
};

SolutionFile.prototype.getProject = function (name) {
	for(var i=0; i<this.projs.length; i++) {
		if(this.projs[i].name==name) {
			return this.projs[i];
		}
	}
	return undefined;
};

exports.SolutionFile = SolutionFile;
