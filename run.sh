
#!/bin/bash

# Variables
MODULE_NAME="rootkit"
MODULE_FILE="${MODULE_NAME}.ko"
MODULE_FILE_PATH="./bin/${MODULE_NAME}.ko"
REMOTE_USER="itamarsch"
REMOTE_HOST="192.168.0.24"
REMOTE_DIR="/tmp" # Directory on the remote machine to transfer the .ko file

read -s -p "Enter sudo password for $REMOTE_USER@$REMOTE_HOST: " SUDO_PASSWORD
echo


# Build the kernel module
echo "Building the kernel module..."
make || { echo "Make failed. Exiting."; exit 1; }

# Check if the module file was created
if [[ ! -f $MODULE_FILE_PATH ]]; then
    echo "Module file $MODULE_FILE not found. Exiting."
    exit 1
fi

# Copy the .ko file to the remote machine
echo "Copying $MODULE_FILE to $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR..."
scp "$MODULE_FILE_PATH" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR" || { echo "SCP failed. Exiting."; exit 1; }

# SSH into the remote machine, insmod the .ko file, and output dmesg
echo "Logging into $REMOTE_HOST to load the module..."
ssh "$REMOTE_USER@$REMOTE_HOST" << EOF
    echo "Switching to $REMOTE_DIR..."
    cd "$REMOTE_DIR" || { echo "Failed to switch to $REMOTE_DIR. Exiting."; exit 1; }
    ls

    echo "$SUDO_PASSWORD" | sudo -S rmmod $MODULE_NAME

    echo "Inserting module $MODULE_FILE..."
    sudo insmod "$MODULE_FILE" || { echo "Failed to insert module. Exiting."; exit 1; }


    echo "Checking kernel messages..."
    sudo dmesg -w
EOF

echo "Operation completed."
