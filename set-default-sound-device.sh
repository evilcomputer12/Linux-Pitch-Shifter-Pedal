#!/bin/bash

# List available playback sound devices
echo "Available sound devices:"
aplay -l

# Get the number of soundcards
card_list=$(aplay -l | grep "^card" | awk -F '[][]' '{print $2}')
echo ""

# Display the list of cards to the user
echo "Please select the default sound device by entering the card number:"
echo "$card_list"
echo ""

# Read user input for the card number
read -p "Enter the card number you want to set as default (e.g., 0, 1, 2, etc.): " card_num

# Check if the input is a valid number
if ! [[ "$card_num" =~ ^[0-9]+$ ]]; then
  echo "Invalid input! Please enter a valid number."
  exit 1
fi

# Ask if the user wants to set the sound device globally or for the current user
read -p "Do you want to set the default sound device globally (for all users)? (y/n): " global_choice

# Define the content for the ALSA config
alsa_config=$(cat << EOF
pcm.!default {
    type hw
    card $card_num
}

ctl.!default {
    type hw
    card $card_num
}
EOF
)

# If the user selects global, update /etc/asound.conf, otherwise update ~/.asoundrc
if [[ "$global_choice" == "y" ]]; then
  echo "Setting default sound device globally (requires sudo)..."
  
  # Use sudo to write the configuration to /etc/asound.conf
  echo "$alsa_config" | sudo tee /etc/asound.conf > /dev/null

  if [[ $? -eq 0 ]]; then
    echo "Global sound device configuration updated in /etc/asound.conf."
  else
    echo "Failed to update /etc/asound.conf. Please try running the script with sudo."
    exit 1
  fi
else
  echo "Setting default sound device for the current user..."
  
  # Update the ~/.asoundrc file for the current user
  asoundrc_path="$HOME/.asoundrc"
  echo "$alsa_config" > "$asoundrc_path"
  echo "User-specific sound device configuration updated in $asoundrc_path."
fi

# Restart ALSA
echo "Restarting ALSA to apply changes..."
sudo alsactl init

echo "Done. The default sound device has been updated."
