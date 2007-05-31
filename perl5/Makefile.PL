use ExtUtils::MakeMaker;
WriteMakefile(
        "NAME"    => "stfl",
	# The -D_LARGEFILE64_SOURCE -D_GNU_SOURCE are needed by perl (not STFL!) on some systems
	"CCFLAGS" => "-I.. -D_LARGEFILE64_SOURCE -D_GNU_SOURCE",
        "LIBS"    => ["-L.. -lstfl -lncursesw"],
        "OBJECT"  => "stfl_wrap.o"
);
