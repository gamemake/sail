var os = require('os');
var path = require('path');
var AppBuilder = require('./builder.js').AppBuilder;
var TaskManager = require('./tasks.js').TaskManager;
var SolutionFile = require('./solution.js').SolutionFile;
var CppToolkit = require('./cpp_toolkit').ClangToolkit;

var solution = new SolutionFile();
var tasks = [];
var config = "debug";
var platform = os.platform();
var is_clean = false;
var is_build = false;

function ParseArgs() {
	for(var i=4; i<process.argv.length; i++) {
		var arg_item = process.argv[i];
		var pos = arg_item.indexOf('=');
		if(pos<0) {
			if(arg_item=='all') {
				for(var p=0; p<solution.projs.length; p++) {
					if(tasks.indexOf(solution.projs[p])<0) {
						tasks.push(solution.projs[p].name);
					}
				}
			} else {
				if(!solution.getProject(arg_item)) {
					console.log('project', arg_item, 'not found');
				} else {
					if(tasks.indexOf(arg_item)<0) {
						tasks.push(arg_item);
					}
				}
			}
		} else {
			var name = arg_item.substring(0, pos);
			var value = arg_item.substring(pos+1);
			switch(name) {
			case 'config':
				config = value;
				break;
			case 'platform':
				platform = value;
				break;
			default:
				return false;
			}
		}
	}

	switch(process.argv[3]) {
	case 'build':
		is_build = true;
		break;
	case 'clean':
		is_clean = true;
		break;
	case 'rebuild':
		is_build = true;
		is_clean = true;
		break;
	default:
		return false;
	}

	return true;
}

function clean(builder) {
	for(var i=0; i<tasks.length; i++) {
		builder.clean(tasks[i]);
	}
}

function build(builder) {
	var start_time = os.uptime();

	for(var i=0; i<tasks.length; i++) {
		builder.buildEXT(tasks[i]);
	}

	builder.manager.build();
	if(builder.verbose) {
		console.log("===== DDL & RPC Depends =====")
		builder.manager.showDepends();
		console.log("=============End=============")
	}
	console.log("==== Building  DDL & RPC ====")
	builder.manager.start(function (error) {
		if(error) {
			console.log('build ddl & rpc error!');
            process.exit(-1);
			return;
		}
		console.log("=============End=============")

		var jobs = builder.jobs;
		builder.jobs = [];
		if(jobs.indexOf('DDLGen')>=0) builder.jobs.push('DDLGen');
		if(jobs.indexOf('RpcGen')>=0) builder.jobs.push('RpcGen');
		builder.manager.reset();

		for(var i=0; i<tasks.length; i++) {
			builder.build(tasks[i]);
		}

		builder.manager.build();
		if(builder.verbose) {
			console.log("===== Projcect  Depends =====");
			builder.manager.showDepends();
			console.log("=============End=============");
		}
		console.log("===== Building  Project =====")
		builder.manager.start(function (error) {
			if(error) {
				console.log('build error');
                process.exit(-1);
				return;
			}
			console.log("=============End=============");

			var use_time = os.uptime() - start_time;
			console.log('done.', use_time, 'seconds');
            process.exit(0);
		});
	});
}

// start main
if(process.argv.length<4) {
	console.log('invalid parameter');
	process.exit(-1);
}

if(!solution.load(process.argv[2])) {
	console.log('failed to load solution file -', process.argv[2]);
	process.exit(-1);
}

if(!ParseArgs()) {
	console.log('invalid parameter.');
	process.exit(-1);
}

var builder = new AppBuilder(solution, new TaskManager(os.cpus().length), new CppToolkit(platform, config));
builder.toolkit.loadConfig(path.join(path.dirname(__filename), 'CppToolkit.json'));

if(is_clean) {
	clean(builder);
}

if(is_build) {
	build(builder);
}
