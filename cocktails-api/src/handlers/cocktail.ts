import prisma from "../db"
import { Request, Response } from "express"
import { AuthRequest } from "../modules/types"

export const getOneCocktail = async (req: Request, res: Response) => {
  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: parseInt(req.params.id)
    },
    include: {
      ingredients: {
        include: {
          ingredient: true
        }
      }
    }
  })

  if (!cocktail) {
    return res.status(404).json({ message: 'Cocktail not found' })
  }

  res.json({ data: cocktail })
}

export const getCocktails = async (req: AuthRequest, res: Response) => {
  const cocktails = await prisma.cocktail.findMany({
    where: {
      userId: req.user.id
    },
    include: {
      ingredients: {
        include: {
          ingredient: true
        }
      }
    }
  })

  res.json({ data: cocktails })
}

export const createCocktail = async (req: AuthRequest, res: Response) => {
  const cocktail = await prisma.cocktail.create({
    data: {
      name: req.body.name,
      category: req.body.category,
      instruction: req.body.instruction,
      userId: req.user.id
    }
  })

  res.json({ data: cocktail })
}

export const updateCocktail = async (req: AuthRequest, res: Response) => {
  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: parseInt(req.params.id)
    }
  })

  if (!cocktail || cocktail.userId !== req.user.id) {
    return res.status(404).json({ message: 'Not found or not authorized' })
  }

  const updatedCocktail = await prisma.cocktail.update({
    where: {
      id: parseInt(req.params.id)
    },
    data: req.body
  })

  res.json({ data: updatedCocktail })
}

export const deleteCocktail = async (req: AuthRequest, res: Response) => {
  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: parseInt(req.params.id)
    }
  })

  if (!cocktail || cocktail.userId !== req.user.id) {
    return res.status(404).json({ message: 'Not found or not authorized' })
  }

  const deleted = await prisma.cocktail.delete({
    where: {
      id: parseInt(req.params.id)
    }
  })

  res.json({ data: deleted })
}

export const addIngredientToCocktail = async (req: AuthRequest, res: Response) => {
  const cocktailId = parseInt(req.params.cocktailId);

  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: cocktailId
    }
  });

  if (!cocktail || cocktail.userId !== req.user.id) {
    return res.status(404).json({ message: 'Cocktail not found or not authorized' });
  }

  try {
    const cocktailIngredient = await prisma.cocktailIngredient.create({
      data: {
        cocktailId,
        ingredientId: parseInt(req.body.ingredientId),
        quantity: req.body.quantity
      },
      include: {
        ingredient: true
      }
    });

    res.json({ data: cocktailIngredient });
  } catch (e) {
    res.status(400).json({ message: 'Could not add ingredient to cocktail' });
  }
};

export const updateCocktailIngredient = async (req: AuthRequest, res: Response) => {
  const cocktailId = parseInt(req.params.cocktailId);
  const ingredientId = parseInt(req.params.ingredientId);

  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: cocktailId
    }
  });

  if (!cocktail || cocktail.userId !== req.user.id) {
    return res.status(404).json({ message: 'Cocktail not found or not authorized' });
  }

  try {
    const updated = await prisma.cocktailIngredient.update({
      where: {
        cocktailId_ingredientId: {
          cocktailId,
          ingredientId
        }
      },
      data: {
        quantity: req.body.quantity
      },
      include: {
        ingredient: true
      }
    });

    res.json({ data: updated });
  } catch (e) {
    res.status(400).json({ message: 'Could not update cocktail ingredient' });
  }
};

export const removeCocktailIngredient = async (req: AuthRequest, res: Response) => {
  const cocktailId = parseInt(req.params.cocktailId);
  const ingredientId = parseInt(req.params.ingredientId);

  const cocktail = await prisma.cocktail.findUnique({
    where: {
      id: cocktailId
    }
  });

  if (!cocktail || cocktail.userId !== req.user.id) {
    return res.status(404).json({ message: 'Cocktail not found or not authorized' });
  }

  try {
    const deleted = await prisma.cocktailIngredient.delete({
      where: {
        cocktailId_ingredientId: {
          cocktailId,
          ingredientId
        }
      }
    });

    res.json({ data: deleted });
  } catch (e) {
    res.status(400).json({ message: 'Could not remove ingredient from cocktail' });
  }
};