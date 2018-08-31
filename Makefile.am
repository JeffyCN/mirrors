AUTOMAKE_OPTIONS = foreign

cardsdir = $(prefix)/usr/share/alsa/cards
cards_DATA = cards/*

install:
	install -D -m 0644 -t $(cardsdir) $(cards_DATA)
