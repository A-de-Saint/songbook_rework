function fitSong(line, minSize, maxSize) {
    let mid = (maxSize + minSize) / 2;
    while (maxSize - minSize > 0.1) {
        mid = (maxSize + minSize) / 2;
        line.style.fontSize = mid + "px";

        if (line.scrollHeigth > line.clientHeight ||
            line.scrollWidth > line.clientWidth) {
            maxSize = mid;
        } else {
            minSize = mid;
        }
    }
    console.log(mid);
}

window.addEventListener("load", () => {
    const nosplit = document.getElementById("nosplit-measurements");
    const nsLines = nosplit.getElementsByClassName("lyrics");
    for (const line of nsLines) {
        fitSong(line, 10, 25);
    }

    const split = document.getElementById("split-measurements");
    const splitLines = split.getElementsByClassName("lyrics");
    for (const line of splitLines) {
        fitSong(line, 10, 25);
    }
});