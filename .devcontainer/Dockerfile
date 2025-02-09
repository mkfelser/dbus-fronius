FROM mcr.microsoft.com/devcontainers/base:debian

RUN apt-get update ; \
    apt-get install qtcreator -y

ADD https://updates.victronenergy.com/feeds/venus/release/sdk/venus-dunfell-x86_64-arm-cortexa8hf-neon-toolchain-v3.54.sh /tmp/

RUN chmod a+x /tmp/venus-dunfell-x86_64-arm-cortexa8hf-neon-toolchain-v3.54.sh ; \
    sudo /tmp/venus-dunfell-x86_64-arm-cortexa8hf-neon-toolchain-v3.54.sh ; \
    sudo ln -s /opt/venus/dunfell-arm-cortexa8hf-neon /opt/venus/current

# Switch to the vscode user
USER vscode

WORKDIR /home/vscode

RUN echo "source /opt/venus/current/environment-setup-cortexa8hf-neon-ve-linux-gnueabi" >> ~/.bashrc 
