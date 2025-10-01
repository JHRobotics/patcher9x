FROM debian:12 AS djgpp

# Install djgpp build dependencies
RUN apt-get update && \
    apt-get install -y bison flex curl gcc g++ make texinfo zlib1g-dev g++ unzip bzip2 xz-utils git

# Download build-djgpp project
RUN git clone --depth 1 https://github.com/andrewwutw/build-djgpp.git && \
    cd build-djgpp && \
    ENABLE_LANGUAGES=c ./build-djgpp.sh 12.2.0


FROM debian:12 AS base

# Install mingw-w64 i686 and x86_64 compilers
RUN apt-get update && \
    apt-get install -y file wget mtools unzip zip gcc gcc-mingw-w64-i686 gcc-mingw-w64-x86-64 fasm make git dos2unix nasm

# Install djgpp
COPY --from=djgpp /usr/local/djgpp /usr/local/djgpp

# Add djgpp to PATH
ENV PATH /usr/local/djgpp/bin:$PATH

# Install FreeDOS LiteUSB and boot floppy images for boot disk
RUN mkdir -p /opt/freedos && \
    wget https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.3/official/FD13-LiteUSB.zip -O /opt/freedos/freedos-liteusb.zip && \
    wget https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.3/official/FD13-FloppyEdition.zip -O /opt/freedos/freedos-floppy.zip

# Download FreeDOS boot templates
RUN mkdir -p /opt/jhr && \
    wget https://files.emulace.cz/dev/freedos-13.ima -O /opt/jhr/freedos-13.ima && \
    wget https://files.emulace.cz/dev/freedos-14.ima -O /opt/jhr/freedos-14.ima

# Grab CWSDPMI binary
RUN mkdir -p /opt/cwsdpmi && \
    wget http://sandmann.dotster.com/cwsdpmi/csdpmi7b.zip -O /opt/cwsdpmi/cwsdpmi.zip

# Assume source in /root
WORKDIR /root
