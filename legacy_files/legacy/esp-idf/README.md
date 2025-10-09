## Getting Started

Ensure you have a developer account/user within the development environment. 
Contact Lucas Aponso to learn more about creating an account.

### Run the following commands to install the esp-idf toolchain:

```bash
mkdir -p ~/.espressif/python_env
cd /opt/esp/esp-idf
IDF_TOOLS_PATH="$HOME/.espressif" \
IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env" \
./install.fish
source /opt/esp/esp-idf/export.fish
echo "set -x IDF_TOOLS_PATH $HOME/.espressif" >> ~/.config/fish/config.fish
echo "set -x IDF_PYTHON_ENV_PATH $HOME/.espressif/python_env" >> ~/.config/fish/config.fish
echo "source /opt/esp/esp-idf/export.fish" >> ~/.config/fish/config.fish
```

### To compile the firmware run the following command in the projects root:

```bash
idf.py build
```

To do a full clean, run the following command:
```bash
idf.py fullclean
```



