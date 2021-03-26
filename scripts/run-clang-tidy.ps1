Get-ChildItem -Path ../source/ -Directory -Recurse |
foreach {
	cd $_.FullName
	&clang-tidy %1 *.cpp *.hpp
}