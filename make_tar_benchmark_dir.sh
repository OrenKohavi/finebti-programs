#!/bin/bash

# Function to create files of a certain size
create_files() {
    local dir=$1
    local num_files=$2
    local file_size=$3
    for i in $(seq 1 $num_files); do
        head -c $file_size </dev/urandom > "$dir/file_$i.txt"
    done
}

# Function to create a directory structure
create_directory_structure() {
    local base=$1
    local depth=$2
    local num_dirs=$3
    local num_files=$4
    local file_size=$5

    if [ $depth -le 0 ]; then
        return
    fi

    mkdir -p "$base"
    create_files "$base" $num_files $file_size

    for i in $(seq 1 $num_dirs); do
        create_directory_structure "$base/dir_$i" $(($depth - 1)) $num_dirs $num_files $file_size
    done
}

# Main script
# Parameters: base directory, depth, number of directories per level, number of files per directory, size of each file in bytes
BASE_DIR="benchmark_directory"
DEPTH=3
NUM_DIRS=10
NUM_FILES=10
FILE_SIZE=$((1 * 1024 * 1024))  # 1 MB per file

# Clean up the old directory if it exists
rm -rf $BASE_DIR

# Create the new directory structure
create_directory_structure $BASE_DIR $DEPTH $NUM_DIRS $NUM_FILES $FILE_SIZE

echo "Benchmark directory structure created at $BASE_DIR"

