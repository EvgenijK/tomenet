FROM registry.fedoraproject.org/fedora:41
RUN dnf -y --setopt=install_weak_deps=False install xorg-x11-server-Xvfb mesa-dri-drivers mesa-libGL libX11 libXext libXcursor libXi libXfixes libXrandr libXScrnSaver libxkbcommon xkeyboard-config python3 strace binutils && dnf clean all
WORKDIR /tmp
