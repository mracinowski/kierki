.PHONY: all clean
.SUFFIXES: tgz

COMMON := -std=c++20 -Wall -Wextra
DEBUG := $(COMMON) -DKIERKI_DEBUG -g3 -Wno-unused-variable -Wno-unused-parameter
RELEASE := $(COMMON) -O2

all: kierki-klient kierki-serwer

###############################################################################
### HEADERS ###################################################################
###############################################################################

### TYPES #####################################################################

TYPES_COMMON := types/common.hpp

TYPES_NUMBER := $(sort types/number.hpp ${TYPES_COMMON})
TYPES_RANK := $(sort types/rank.hpp ${TYPES_COMMON})
TYPES_SCORE := $(sort types/score.hpp ${TYPES_COMMON})
TYPES_SEAT := $(sort types/seat.hpp ${TYPES_COMMON})
TYPES_SUIT := $(sort types/suit.hpp ${TYPES_COMMON})

TYPES_SCORES := $(sort types/scores.hpp ${TYPES_COMMON} ${TYPES_SCORES})
TYPES_SEATS := $(sort types/seats.hpp ${TYPES_COMMON} ${TYPES_SEAT})
TYPES_CARD := $(sort types/card.hpp ${TYPES_COMMON} ${TYPES_RANK} ${TYPES_SUIT})

TYPES_CARDS := $(sort types/cards.hpp ${TYPES_CARD} ${TYPES_COMMON})
TYPES_HAND := $(sort types/hand.hpp ${TYPES_CARD} ${TYPES_COMMON})
TYPES_TRICK := $(sort types/trick.hpp ${TYPES_COMMON} ${TYPES_CARD})

TYPES_RULESET := $(sort types/ruleset.hpp ${TYPES_COMMON} ${TYPES_CARD} ${TYPES_TRICK} ${TYPES_NUMBER} ${TYPES_SCORE})

TYPES_ROUND := $(sort types/round.hpp ${TYPES_COMMON} ${TYPES_TRICK} ${TYPES_RULESET} ${TYPES_SEAT} ${TYPES_HAND})

TYPES_MESSAGE := $(sort types/message.hpp ${TYPES_CARDS} ${TYPES_COMMON} ${TYPES_NUMBER} ${TYPES_RULESET} ${TYPES_SEATS} ${TYPES_SCORES})

### IO ########################################################################

IO_CHANNEL := io/channel.hpp

IO_BROKER := $(sort io/broker.hpp \
	${IO_CHANNEL} \
)

IO_INPUT := $(sort io/input.hpp \
	${IO_CHANNEL} \
)

IO_OUTPUT := $(sort io/output.hpp \
	${IO_CHANNEL} \
)

### TCP #######################################################################

IO_TCP_ADDRESS := io/tcp/address.hpp

IO_TCP_CONNECTION := $(sort io/tcp/connection.hpp \
	${IO_CHANNEL} \
	${IO_INPUT} \
	${IO_OUTPUT} \
)

IO_TCP_CLIENT := $(sort io/tcp/client.hpp \
	${IO_CHANNEL} \
	$(IO_TCP_ADDRESS) \
	${IO_TCP_CONNECTION} \
)

IO_TCP_SERVER := $(sort io/tcp/server.hpp \
	${IO_CHANNEL} \
	$(IO_TCP_ADDRESS) \
	${IO_TCP_CONNECTION} \
)

### ... #######################################################################

COMMON := common.hpp

CONFIG := config.hpp

REPORT := report.hpp

ENGINES_ENGINE := $(sort engines/engine.hpp ${TYPES_MESSAGE})
PLAYERS_PLAYER := $(sort players/player.hpp ${TYPES_MESSAGE} ${ENGINES_ENGINE})
ENGINES_ENGINE := $(sort ${ENGINES_ENGINE} ${PLAYERS_PLAYER})

### ENGINES ###################################################################

ENGINES_LOCAL := $(sort engines/local.hpp \
	${TYPES_ROUND} \
	${ENGINES_ENGINE} \
)

ENGINES_REMOTE := $(sort engines/remote.hpp \
	${IO_INPUT} \
	${IO_OUTPUT} \
	${REPORT} \
	${TYPES_MESSAGE} \
	${ENGINES_ENGINE} \
)

### PLAYERS ###################################################################

PLAYERS_AUTOMATIC := $(sort players/automatic.hpp \
	${PLAYERS_PLAYER} \
)

PLAYERS_INTERACTIVE := $(sort players/interactive.hpp \
	${IO_INPUT} \
	${IO_OUTPUT} \
	${PLAYERS_PLAYER} \
)

PLAYERS_REMOTE := $(sort players/remote.hpp \
	${IO_INPUT} \
	${IO_OUTPUT} \
	${TYPES_MESSAGE} \
)

### EXECUTABLES ###############################################################

KLIENT := $(sort \
	${COMMON} \
	${CONFIG} \
	${IO_BROKER} \
	${IO_TCP_CLIENT} \
	${ENGINES_REMOTE} \
	${PLAYERS_AUTOMATIC} \
	${PLAYERS_INTERACTIVE} \
)

SERWER := $(sort \
	${COMMON} \
	${CONFIG} \
	${IO_BROKER} \
	${IO_TCP_SERVER} \
	${ENGINES_LOCAL} \
	${PLAYERS_REMOTE} \
)

###############################################################################
### RULES #####################################################################
###############################################################################

debug-klient kierki-klient: kierki-klient.cpp debug.hpp
debug-serwer kierki-serwer: kierki-serwer.cpp debug.hpp

debug-klient kierki-klient: $(KLIENT)
debug-serwer kierki-klient: $(SERWER)

kierki-klient kierki-serwer:
	g++ $(RELEASE) -o $@ $(filter %.cpp,$^)

debug-klient debug-serwer:
	g++ $(DEBUG) -fdiagnostics-color=always -o $@ $(filter %.cpp,$^) 2>&1 | less -FR

clean:
	-rm debug-klient debug-serwer kierki-klient kierki-serwer

