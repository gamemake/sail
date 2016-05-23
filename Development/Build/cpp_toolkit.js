
var fs = require('fs');
var path = require('path')
var util = require("util");

if(!String.prototype.trim) {
	String.prototype.trim = function () {
		return this.replace(/^\s+|\s+$/g, '');
	};
}

if(!String.prototype.ltrim) {
	String.prototype.ltrim = function () {
		return this.replace(/^\s+/, '');
	};
}

if(!String.prototype.rtrim) {
	String.prototype.rtrim = function () {
		return this.replace(/\s+$/, '');
	};
}

if(!String.prototype.fulltrim) {
	String.prototype.fulltrim = function () {
		return this.replace(/(?:(?:^|\n)\s+|\s+(?:$|\n))/g,'').replace(/\s+/g, ' ');
	};
}

if(!String.prototype.format) {
	String.prototype.format = function() {
		var args = arguments;
		return this.replace(/{(\d+)}/g, function(match, number) {
			return typeof args[number] != 'undefined' ? args[number] : match ;
		});
	};
}

function CppToolkit(platform, config) {
	this.platform = platform;
	this.config = config;
	this.options = {};
	
	this.obj_flags = [];
	this.lib_flags = [];
	this.dll_flags = [];
	this.exe_flags = [];
}

CppToolkit.prototype.loadConfig = function (file) {
	try {
		var json_text = fs.readFileSync(file);
		this.options = JSON.parse(json_text);
	} catch(e) {
		console.log('failed to load', file, 'error =', e.message);
		return false;
	}
	return true;
}

CppToolkit.prototype.getOption = function (name, deps) {
	var filter = [].concat(deps);
	filter.push(this.platform);
	filter.push(this.config);
	
	var ret_val = [];
	for(var vname in this.options) {
		var vnames = vname.split('.');
		if(vnames[0]!=name) continue;
		var i = 1;
		for(; i<vnames.length; i++) {
			if(filter.indexOf(vnames[i])<0) break;
		}
		if(i!=vnames.length) continue;
		ret_val = ret_val.concat(this.options[vname]);
	}
	
	return ret_val;
};

CppToolkit.prototype.checkExtDeps = function (deps) {
	var third_deps = this.getOption("third_deps", []);
	for(var i=0; i<deps.length; i++) {
		if(third_deps.indexOf(deps[i])<0) {
			console.log('error', deps, 'not in', third_deps);
			return false;
		}
	}
	return true;
}

CppToolkit.prototype.genObjectFile = function (out_dir, filename) {
	var vfilename = filename.replace(/\\/g, "_").replace(/\//g, "_").replace(/\./g, "_");
	return path.join(out_dir, vfilename) + (this.platform=='win32'?".obj":".o");
};

CppToolkit.prototype.genStaticLibrary = function (out_dir, proj_name) {
	if(this.platform=='win32') {
		return path.join(out_dir, proj_name) + ".lib";
	} else {
		return path.join(out_dir, 'lib' + proj_name) + ".a";
	}
};

CppToolkit.prototype.genSharedLibrary = function (out_dir, proj_name) {
	if(this.platform=='win32') {
		return path.join(out_dir, proj_name) + ".dll";
	} else {
		return path.join(out_dir, proj_name) + ".so";
	}
};

CppToolkit.prototype.genExecuteFile = function (out_dir, proj_name) {
	if(this.platform=='win32') {
		return path.join(out_dir, proj_name) + ".exe";
	} else {
		return path.join(out_dir, proj_name);
	}
};

CppToolkit.prototype.compile = function (out_file, src_file, in_incs, in_defs) {
};

CppToolkit.prototype.compileDefine = function (out_file, src_file, in_incs, in_defs) {
};

CppToolkit.prototype.buildStaticLibrary = function (out_file, objs) {
};

CppToolkit.prototype.buildSharedLibrary = function (out_file, objs, libs, libs_inc) {
};

CppToolkit.prototype.buildProgram = function (out_file, objs, libs, libs_inc) {
};

function ClangToolkit(platform, config) {
	CppToolkit.call(this, platform, config);
}
util.inherits(ClangToolkit, CppToolkit);

ClangToolkit.prototype.isSupport = function (src_file) {
	var ext_name = path.extname(src_file);
	if(ext_name=='.c' || ext_name=='.cpp' || ext_name=='.cc') return true;
	if((ext_name=='.m' || ext_name=='.mm') && this.platform=='darwin') return true;
	return false;
}

ClangToolkit.prototype.compile = function (out_file, src_file, in_incs, in_defs, ext_deps) {
	var flags = [];
	var incs = [].concat(this.getOption("incs", ext_deps)).concat(in_incs);
	var defs = [].concat(this.getOption("defs", ext_deps)).concat(in_defs);
	var extname = path.extname(src_file);

	var cmd = 'clang++';
	if(extname=='.c') {
		cmd = 'clang';
		flags = flags.concat(this.getOption('cc_flags', ext_deps));
	} else {
		flags = flags.concat(this.getOption('cpp_flags', ext_deps));
	}
	var args = [].concat(flags);
	for(var i=0; i<incs.length; i++) {
		args.push('-I' + incs[i]);
	}
	for(var i=0; i<defs.length; i++) {
		args.push('-D' + defs[i]);
	}
	if(extname=='.m' || extname=='.mm') {
		var frameworks = [].concat(this.getOption("frameworks", ext_deps));
		for(var i=0; i<frameworks.length; i++) {
			args.push('-framework');
			args.push(frameworks[i]);
		}
	}
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push(src_file);
	args.push("-c");
	args.push("-o");
	args.push(out_file);

	return { "cmd": cmd, "args": args };
};

ClangToolkit.prototype.compileDefine = function (out_file, src_file, in_incs, in_defs, ext_deps) {
	var cmd = this.compile(out_file, src_file, in_incs, in_defs, ext_deps);
	var pos = cmd.args.length - 3;
	cmd.args[pos+0] = "-M";
	cmd.args[pos+1] = "-MF";
	return cmd;
};

ClangToolkit.prototype.buildStaticLibrary = function (out_file, objs) {
	return { "cmd": "ar", "args": ["rcs", out_file].concat(objs) };
};

ClangToolkit.prototype.buildSharedLibrary = function (out_file, objs, in_libs, in_lib_dirs, ext_deps) {
	var flags = [].concat(this.getOption('lib_flags', ext_deps));
	var libs = [].concat(this.getOption("libs", ext_deps)).concat(in_libs);
	var lib_dirs = [].concat(this.getOption("lib_dirs", ext_deps)).concat(in_lib_dirs);

	var args = [].concat(flags);
	
	for(var i=0; i<libs.length; i++) {
		args.push('-l' + libs[i]);
	}
	for(var i=0; i<lib_dirs.length; i++) {
		args.push('-L' + lib_dirs[i]);
	}
	if(this.platform=='darwin') {
		var frameworks = [].concat(this.getOption("frameworks", ext_deps));
		for(var i=0; i<frameworks.length; i++) {
			args.push('-framework' + frameworks[i]);
		}
	}
	for(var i=0; i<objs.length; i++) {
		args.push(objs[i]);
	}
	args.push("-shared");
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push('-o');
	args.push(out_file);

	return { "cmd": "clang++", "args": args };
};

ClangToolkit.prototype.buildProgram = function (out_file, objs, in_libs, in_lib_dirs, ext_deps) {
	var flags = [].concat(this.getOption('lib_flags', ext_deps));
	var libs = [].concat(this.getOption("libs", ext_deps)).concat(in_libs);
	var lib_dirs = [].concat(this.getOption("lib_dirs", ext_deps)).concat(in_lib_dirs);
	var frameworks = [].concat(this.getOption("frameworks", ext_deps));

	var args = [].concat(flags);
	
	for(var i=0; i<libs.length; i++) {
		args.push('-l' + libs[i]);
	}
	for(var i=0; i<lib_dirs.length; i++) {
		args.push('-L' + lib_dirs[i]);
	}
	for(var i=0; i<frameworks.length; i++) {
		args.push('-framework');
		args.push(frameworks[i]);
	}
	for(var i=0; i<objs.length; i++) {
		args.push(objs[i]);
	}
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push('-o');
	args.push(out_file);

	return { "cmd": "clang++", "args": args };
};

function VCToolkit(platform, config) {
	CppToolkit.call(this, platform, config);
}
util.inherits(VCToolkit, CppToolkit);

VCToolkit.prototype.isSupport = function (src_file) {
	var ext_name = path.extname(src_file);
	if(ext_name=='.c' || ext_name=='.cpp' || ext_name=='.cc') return true;
	return false;
}

VCToolkit.prototype.compile = function (out_file, src_file, in_incs, in_defs, ext_deps) {
	var flags = [];
	var incs = [].concat(this.getOption("incs", ext_deps)).concat(in_incs);
	var defs = [].concat(this.getOption("defs", ext_deps)).concat(in_defs);

	var cmd = 'clang++';
	if(path.extname(src_file)=='.c') {
		cmd = 'clang';
		flags = flags.concat(this.getOption('cc_flags', ext_deps));
	} else {
		flags = flags.concat(this.getOption('cpp_flags', ext_deps));
	}
	var args = [].concat(flags);
	for(var i=0; i<incs.length; i++) {
		args.push('-I' + incs[i]);
	}
	for(var i=0; i<defs.length; i++) {
		args.push('-D' + defs[i]);
	}
	if(this.platform=='darwin') {
		var frameworks = [].concat(this.getOption("frameworks", ext_deps));
		for(var i=0; i<frameworks.length; i++) {
			args.push('-framework' + frameworks[i]);
		}
	}
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push(src_file);
	args.push("-c");
	args.push("-o");
	args.push(out_file);

	return { "cmd": cmd, "args": args };
};

VCToolkit.prototype.compileDefine = function (out_file, src_file, in_incs, in_defs, ext_deps) {
	var cmd = this.compile(out_file, src_file, in_incs, in_defs, ext_deps);
	var pos = cmd.length - 3;
	cmd.args[pos+0] = "-M";
	cmd.args[pos+1] = "-MF";
	cmd.args[pos+2] += ".d";
	return cmd;
};

VCToolkit.prototype.buildStaticLibrary = function (out_file, objs) {
	return { "cmd": "ar", "args": ["rcs", out_file].concat(objs) };
};

VCToolkit.prototype.buildSharedLibrary = function (out_file, objs, in_libs, in_lib_dirs, ext_deps) {
	var libs = [].concat(this.getOption("libs", ext_deps)).concat(in_libs);
	var lib_dirs = [].concat(this.getOption("lib_dirs", ext_deps)).concat(in_lib_dirs);

	var args = [];
	
	for(var i=0; i<libs.length; i++) {
		args.push('-l' + libs[i]);
	}
	for(var i=0; i<lib_dirs.length; i++) {
		args.push('-L' + lib_dirs[i]);
	}
	if(this.platform=='darwin') {
		var frameworks = [].concat(this.getOption("frameworks", ext_deps));
		for(var i=0; i<frameworks.length; i++) {
			args.push('-framework' + frameworks[i]);
		}
	}
	for(var i=0; i<objs.length; i++) {
		args.push(objs[i]);
	}
	args.push("-shared");
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push('-o');
	args.push(out_file);

	return { "cmd": "clang++", "args": args };
};

VCToolkit.prototype.buildProgram = function (out_file, objs, in_libs, in_lib_dirs, ext_deps) {
	var libs = [].concat(this.getOption("libs", ext_deps)).concat(in_libs);
	var lib_dirs = [].concat(this.getOption("lib_dirs", ext_deps)).concat(in_lib_dirs);
	var frameworks = [].concat(this.getOption("frameworks", ext_deps));

	var args = [];
	
	for(var i=0; i<libs.length; i++) {
		args.push('-l' + libs[i]);
	}
	for(var i=0; i<lib_dirs.length; i++) {
		args.push('-L' + lib_dirs[i]);
	}
	for(var i=0; i<frameworks.length; i++) {
		args.push('-framework' + frameworks[i]);
	}
	for(var i=0; i<objs.length; i++) {
		args.push(objs[i]);
	}
	if(this.config=='debug') {
		args.push('-g');
	}
	args.push('-o');
	args.push(out_file);

	return { "cmd": "clang++", "args": args };
};

exports.CppToolkit = CppToolkit;
exports.ClangToolkit = ClangToolkit;
exports.VCToolkit = VCToolkit;
