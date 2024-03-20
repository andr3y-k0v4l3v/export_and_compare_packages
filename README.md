# Description
____
A utility that exports information about packages in different branches and compares them. We also output the result of the work to a JSON file. This utility compares packages in three directions:
- all packages that are in the first branch, but not in the second
- all packages that are in the second branch, but not in the first
- all version-release packages of which there are more in the 1st than in the 2nd

# Install dependencies
____

``` sh
sudo make setup
```

# Build utility
____

``` sh
sudo make
```

# Install utility
____

``` sh
sudo make install
```

# Uninstall utility
____

``` sh
sudo make uninstall
```
