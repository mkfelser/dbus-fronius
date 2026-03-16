FROM mcr.microsoft.com/devcontainers/base:debian

RUN apt-get update ; \
    apt-get install file qtcreator -y

ADD https://updates.victronenergy.com/feeds/venus/release/sdk/venus-scarthgap-x86_64-arm-cortexa8hf-neon-toolchain-v3.64.sh /tmp/

RUN chmod a+x /tmp/venus-scarthgap-x86_64-arm-cortexa8hf-neon-toolchain-v3.64.sh ; \
    sudo /tmp/venus-scarthgap-x86_64-arm-cortexa8hf-neon-toolchain-v3.64.sh ; \
    sudo ln -s /opt/venus/scarthgap-arm-cortexa8hf-neon /opt/venus/current
    
# Switch to the vscode user
USER vscode

WORKDIR /home/vscode

RUN echo "source /opt/venus/current/environment-setup-cortexa8hf-neon-ve-linux-gnueabi" >> ~/.bashrc 
RUN echo "export PATH=\$PATH:/opt/venus/current/sysroots/x86_64-ve-linux/usr/bin/qt6" >> ~/.bashrc