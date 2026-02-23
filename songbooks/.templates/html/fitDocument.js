window.addEventListener("load", () => {
    const songs = document.getElementsByClassName("song-content");
    for (const song of songs) {
        let fontSize = parseFloat(window.getComputedStyle(song).fontSize);
        fontSize = Math.min(fontSize, 23.0); //max fontsize shall be 23.0 px
        fitSong(song, 10, fontSize);
    }
});

function fitSong(song, minSize, maxSize) {
    let size = maxSize;
    const lines = song.getElementsByClassName("lyrics");
    for (const line of lines) {
        let max = maxSize + 0.2;
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
        if (mid < size) {
            size = mid;
        }
    }
    song.style.fontSize = size + "px";
}