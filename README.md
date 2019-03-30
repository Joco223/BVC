# BVC

Better Version Control. System for local file backup.

This program is used for local file backup and keepeing it up to date. It scans the source directories and the target directory and checks for differences between files and directories. If there is any it updates the files in target directory with files from source directories. Difference existance is checked by comparing sha256 of files.

For configuration, it reads the config file (passed in with command line parameter, so `BVC.exe config.cfg`, necessary parameters are at least one `source` and only one `destination`. Optional parameter is `ignored` which can be used to ignore files with a defined file extension. This is what it should look like:

source = C:/Programming/C++/
destination = D:/Backup/Programming/C++/

ignored = exe
^ With this it will ignore any files with .exe extension

It will also create any missing directories up to target directory. You can add -l flag for more verbose output. It would output all scanned directories, all checked files and a representation of source directories trees.