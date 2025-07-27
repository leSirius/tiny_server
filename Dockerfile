FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    unzip \
    software-properties-common \
    openssh-server \
    rsync \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir /var/run/sshd
RUN echo 'root:secure' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/UsePAM yes/UsePAM no/' /etc/ssh/sshd_config
EXPOSE 22

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs)-20 main"
RUN apt-get update

RUN CMAKE_VERSION="3.31.0" && \
    CMAKE_ARCH="linux-aarch64" && \
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.tar.gz" && \
    mkdir -p /opt/cmake && \
    wget -qO- ${CMAKE_URL} | tar -xz --strip-components=1 -C /opt/cmake && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

RUN wget https://github.com/ninja-build/ninja/releases/download/v1.13.1/ninja-linux-aarch64.zip && \
    unzip ninja-linux-aarch64.zip -d /usr/local/bin && \
    chmod +x /usr/local/bin/ninja && \
    rm ninja-linux-aarch64.zip

RUN apt-get update && apt-get install -y \
    clang-20 \
    libc++-20-dev \
    libc++abi-20-dev \
    clang-tools-20 \
    gdb \
    make \
    --no-install-recommends \
    && rm -rf /var/lib/apt/lists/*


RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-20 100

WORKDIR /app
#COPY . .

CMD ["/usr/sbin/sshd", "-D"]
#CMD ["/bin/bash"]
