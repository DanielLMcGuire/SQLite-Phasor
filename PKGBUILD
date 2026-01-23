pkgname=sqlite-Phasor
pkgver=0.1.0
pkgrel=1
pkgdesc="SQLite Bindings for Phasor"
arch=('x86_64')
url="https://github.com/DanielLMcGuire/SQLite-Phasor"
license=('MIT')
depends=('cmake' 'ninja')
makedepends=('git' 'gcc')
source=("git+https://github.com/DanielLMcGuire/SQLite-Phasor.git")
sha256sums=('SKIP')

pkgver() {
    cd "$srcdir/SQLite-Phasor"
    tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
    commits_since_tag=$(git rev-list "${tag}"..HEAD --count 2>/dev/null || echo 0)
    if [ "$commits_since_tag" -eq 0 ]; then
        echo "$tag"
    else
        echo "${tag}.r${commits_since_tag}"
    fi
}


build() {
	cd "$srcdir/SQLite-Phasor"
    cmake -S "$srcdir/SQLite-Phasor" -B "$srcdir/SQLite-Phasor/build" \
          -G Ninja \
          -DCMAKE_INSTALL_PREFIX=/opt/Phasor
    cmake --build "$srcdir/SQLite-Phasor/build"
}

package() {
    cd "$srcdir/SQLite-Phasor/build"
    DESTDIR="$pkgdir" cmake --install .

    src="$srcdir/SQLite-Phasor/man3"
    dest="$pkgdir/usr/share/man/man3"
    
    if [ -d "$src" ]; then
        mkdir -p "$dest"
        for file in "$src"/*.3; do
            [ -f "$file" ] && install -Dm644 "$file" "$dest/"
        done
    fi
}