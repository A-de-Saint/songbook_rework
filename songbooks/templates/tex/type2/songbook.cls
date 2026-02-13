\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{songbook} [2026/02/11 Default songbook class]

\LoadClass[12pt]{article}

\RequirePackage{geometry}
\RequirePackage{inconsolata}
\RequirePackage{setspace}
\RequirePackage{graphicx}

\geometry{
  a4paper,
  margin=2cm
}

% Commands for songbook name and songbook author %
\newcommand{\songbookname}[1]{
  \Huge\bfseries{{#1}\par}%
}
\newcommand{\songbookauthor}[1]{
  \Large{{#1}\par}%
}

% a command for song title %
% example: \songtitle{Zafúkané Fleret} %
\newcommand{\songtitle}[1]{
  \begin{center}
    {\Large\bfseries #1\par}
  \end{center}
  \addcontentsline{toc}{section}{#1}
}

% sets the song's size to parameters %
% example: {\songsize{12pt}{10pt}  //song }
\newcommand{\songsize}[2]{
  \fontsize{#1}{#2}\selectfont
}

% general song looks %
\newenvironment{song}{
    \par
    \begin{center}
    \begin{minipage}{\textwidth}
    \ttfamily
}{
    %\end{alltt}
    \end{minipage}
    \end{center}
    \par
    \newpage
}

% optional enverinments (empty by default) %
% change at your own risk %
\newenvironment{song_verse}{}{\vspace{\baselineskip}}
\newenvironment{song_chorus}{}{\vspace{\baselineskip}}
\newenvironment{song_intro}{}{\vspace{\baselineskip}}
\newenvironment{song_ending}{}{\vspace{\baselineskip}}
\newenvironment{song_interlude}{}{\vspace{\baselineskip}}
\newenvironment{song_bridge}{}{\vspace{\baselineskip}}
\newenvironment{song_other}{}{\vspace{\baselineskip}}

% commands for chords and lyrics %
\newcommand{\chord}[1]{%
  \raisebox{1ex}{\textbf{#1}}%
}
\newcommand{\lyrics}[1]{#1\par}

\setlength{\parindent}{0pt}