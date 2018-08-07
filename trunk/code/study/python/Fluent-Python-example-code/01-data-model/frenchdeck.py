import collections
from random import choice
suit_values = dict(spades=3, hearts=2, diamonds=1, clubs=0)
Card = collections.namedtuple('Card', ['rank', 'suit'])

class FrenchDeck:
    ranks = [str(n) for n in range(2, 11)] + list('JQKA')
    suits = 'spades diamonds clubs hearts'.split()

    def __init__(self):
        self._cards = [Card(rank, suit) for suit in self.suits
                                        for rank in self.ranks]

    def __len__(self):
        return len(self._cards)

    def __getitem__(self, position):
        return self._cards[position]

def spades_higt(card):
    rank_value=FrenchDeck.ranks.index(card.rank)
    return rank_value*len(suit_values) + suit_values[card.suit]


if __name__=="__main__":
    beer_card=Card('7','hearts')
    #print(beer_card)
    deck=FrenchDeck()
    for n, card in enumerate(deck, 1):
        print(n, card)
    print("----------------------------------------------")
    for card in sorted(deck, key=spades_higt):
        print card
