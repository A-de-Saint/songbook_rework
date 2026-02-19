window.addEventListener("load", () => {
    const songs = document.getElementsByClassName("song-content");
    for (const song of songs) {
        const fontSize = window.getComputedStyle(song).fontSize;
        fitSong(song, 10, fontSize);
    }
});

function fitSong(song, maxSize, minSize) {
    let size = maxSize;
    const lines = song.getElementsByClassName("lyrics");
    for (const line of lines) {
        let max = size + 0.2;
        let min = minSize;
        let mid = (max + min) / 2;
        while (max - min >= 0.2) {
            mid = (max + min) / 2;
            line.style.fontSize = mid + "px";

            if (line.scrollWidth > line.clientWidth) {
                max = mid;
            } else {
                min = mid;
            }
        }
        if (mid > size) {
            size = mid;
        }
    }
    song.style.fontSize = size + "px";
    for (const line of lines) {
        line.style.removeProperty("font-size");
    }
}