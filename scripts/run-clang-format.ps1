Get-ChildItem -Path ../source/ -Directory -Recurse |
foreach {
	cd $_.FullName
	&clang-format -i *.cpp *.hpp
}