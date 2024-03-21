# Description
A utility that exports information about packages in different branches and compares them. We also output the result of the work to a JSON file. This utility compares packages in three directions:
- all packages that are in the first branch, but not in the second
- all packages that are in the second branch, but not in the first
- all version-release packages of which there are more in the 1st than in the 2nd

# Install C++

``` sh
sudo apt-get install gcc-c++
```

# Install dependencies

``` sh
sudo make setup
```

# Build utility

``` sh
sudo make
```

# Install utility

``` sh
sudo make install
```

# Uninstall utility

``` sh
sudo make uninstall
```

# Use utility

``` sh
cmp_packages <name_branch1> <name_branch2> <output_json_file>
```

