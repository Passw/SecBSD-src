# XXX Configure test needed?
# Some SecBSDs seem to have a dlopen() that won't accept relative paths
$self->{CCFLAGS} = $Config{ccflags} . ' -DDLOPEN_WONT_DO_RELATIVE_PATHS';
