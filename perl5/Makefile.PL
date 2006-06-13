use ExtUtils::MakeMaker;
WriteMakefile(
        "NAME"    => "stfl",
	"CCFLAGS" => "-I..",
        "LIBS"    => ["-L.. -lstfl -lncurses"],
        "OBJECT"  => "stfl_wrap.o"
);
