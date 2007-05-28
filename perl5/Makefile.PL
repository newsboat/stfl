use ExtUtils::MakeMaker;
WriteMakefile(
        "NAME"    => "stfl",
	"CCFLAGS" => "-I..",
        "LIBS"    => ["-L.. -lstfl -lncursesw"],
        "OBJECT"  => "stfl_wrap.o"
);
