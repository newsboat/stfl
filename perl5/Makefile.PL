use ExtUtils::MakeMaker;
WriteMakefile(
        "NAME"    => "stfl",
        "LIBS"    => ["-L.. -lstfl -lncurses"],
        "OBJECT"  => "stfl_wrap.o"
);
