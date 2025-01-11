#!/bin/bash

# Variables
MODULE_NAME="rootkit"
MODULE_FILE="${MODULE_NAME}.ko"
MODULE_FILE_PATH="./bin/${MODULE_NAME}.ko"
REMOTE_USER="itamarsch"
REMOTE_HOST="192.168.0.99"
REMOTE_DIR="/tmp" # Directory on the remote machine to transfer the .ko file

# Color codes
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RESET="\033[0m"

read -s -p "$(echo -e "${YELLOW}Enter sudo password for $REMOTE_USER@$REMOTE_HOST:${RESET} ")" SUDO_PASSWORD
echo

# Build the kernel module
echo -e "${BLUE}Building the kernel module...${RESET}"
make || {
  echo -e "${RED}Make failed. Exiting.${RESET}"
  exit 1
}

# Check if the module file was created
if [[ ! -f $MODULE_FILE_PATH ]]; then
  echo -e "${RED}Module file $MODULE_FILE not found. Exiting.${RESET}"
  exit 1
fi

# Copy the .ko file to the remote machine
echo -e "${BLUE}Copying $MODULE_FILE to $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR...${RESET}"
scp "$MODULE_FILE_PATH" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR" || {
  echo -e "${RED}SCP failed. Exiting.${RESET}"
  exit 1
}

# SSH into the remote machine, insmod the .ko file, and output dmesg
echo -e "${BLUE}Logging into $REMOTE_HOST to load the module...${RESET}"
ssh "$REMOTE_USER@$REMOTE_HOST" <<EOF
    echo -e "${BLUE}Switching to $REMOTE_DIR...${RESET}"
    cd "$REMOTE_DIR" || { echo -e "${RED}Failed to switch to $REMOTE_DIR. Exiting.${RESET}"; exit 1; }

    echo "$SUDO_PASSWORD" | sudo -S rmmod $MODULE_NAME

    echo -e "${GREEN}Inserting module $MODULE_FILE...${RESET}"
    sudo insmod "$MODULE_FILE" || { echo -e "${RED}Failed to insert module. Exiting.${RESET}"; exit 1; }

    echo -e "${BLUE}Checking kernel messages...${RESET}"
    sudo dmesg --clear
    sudo dmesg -w
EOF

echo -e "${GREEN}Operation completed.${RESET}"
