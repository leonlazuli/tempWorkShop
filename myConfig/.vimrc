syntax on
syntax enable
highlight Pmenu ctermbg=White 
"set background=dark
"colorscheme solarized
set shiftwidth=4
set tabstop=4
set ts=4
set expandtab
set softtabstop=4
set autoindent
set smartindent
set cindent
set noswapfile
set mouse=a
set hlsearch
set incsearch
let Tlist_Show_One_File=1  
let Tlist_Exit_OnlyWindow=1 

"shift tab pages
map <S-left> :tabp<cr>
map <S-right> :tabn<cr>
map <C-S> :w<cr>
"noremap j k
"noremap k j


"auto complete {
inoremap { {<CR>}<Esc>ko

"Leon's set
set pastetoggle=<f5>
set ignorecase
set smartcase

"Leon's set end    

"Leon's shortcut
cnoremap :lpaste<CR> :set paste<CR><Esc>set mouse -=a<CR> 
cnoremap :lnpaste<CR> :set nopaste<CR><Esc>set mouse=a<CR> 


"Leon's shortcut end

"Leon's keymap
noremap <Up> <Nop>
noremap <Down> <Nop>
noremap <Left> <Nop>
noremap <Right> <Nop>
inoremap <silent> <C-n> <Esc>:nohl<CR>a
nnoremap <silent> <C-l> :<C-u>nohlsearch<CR><C-l>

"Leon's keymap end 

set rtp+=~/.vim/bundle/vundle/
call vundle#rc()

"for Vundle Vundle
Bundle 'gmarik/vundle'
"NEVER Delete the above line

Bundle 'Valloric/YouCompleteMe'
"for YCM
let g:ycm_autoclose_preview_window_after_completion = 1
let g:ycm_collect_identifiers_from_comments_and_strings = 0
let g:ycm_complete_in_strings = 0

"for Vundle End
