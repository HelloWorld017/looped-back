{
	"targets": [
		{
			"target_name": "LoopedBack",
			"sources": [ "src/LoopedBack.cc" ],
			"include_dirs" : [
				"<!(node -e \"require('nan')\")"
			],
			"msvs_settings": {
				"VCCLCompilerTool": {
					"ExceptionHandling": 1
				}
			}
		}
	]
}
