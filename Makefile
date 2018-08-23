
CARDS = cards/aliases.conf \
	cards/rk817.conf \
	cards/rt5640.conf \
	cards/rt5651.conf

install:
	install -D -m 0644 -t $(DESTDIR)/usr/share/alsa/cards/ $(CARDS)
