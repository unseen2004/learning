import request from 'supertest';
import app from '../../src/server';
import { testUser } from '../fixtures/users';
import { mojito } from '../fixtures/cocktails';
import { mint, rum } from '../fixtures/ingredients';

describe('Cocktail-Ingredient Relationship API', () => {
  let token: string;
  let cocktailId: number;
  let mintId: number;
  let rumId: number;

  beforeEach(async () => {
    const authRes = await request(app)
      .post('/user')
      .send(testUser);

    token = authRes.body.token;

    const cocktailRes = await request(app)
      .post('/api/cocktail')
      .set('Authorization', `Bearer ${token}`)
      .send(mojito);

    cocktailId = cocktailRes.body.data.id;

    const mintRes = await request(app)
      .post('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .field('name', mint.name)
      .field('description', mint.description)
      .field('alcohol', mint.alcohol);

    mintId = mintRes.body.data.id;

    const rumRes = await request(app)
      .post('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .field('name', rum.name)
      .field('description', rum.description)
      .field('alcohol', rum.alcohol);

    rumId = rumRes.body.data.id;
  });

  it('should add an ingredient to a cocktail', async () => {
    const res = await request(app)
      .post(`/api/cocktail/${cocktailId}/ingredient`)
      .set('Authorization', `Bearer ${token}`)
      .send({
        ingredientId: mintId,
        quantity: '10 leaves'
      })
      .expect(200);

    expect(res.body.data.cocktailId).toBe(cocktailId);
    expect(res.body.data.ingredientId).toBe(mintId);
    expect(res.body.data.quantity).toBe('10 leaves');
  });

  it('should update a cocktail ingredient', async () => {
    await request(app)
      .post(`/api/cocktail/${cocktailId}/ingredient`)
      .set('Authorization', `Bearer ${token}`)
      .send({
        ingredientId: mintId,
        quantity: '10 leaves'
      });

    const res = await request(app)
      .put(`/api/cocktail/${cocktailId}/ingredient/${mintId}`)
      .set('Authorization', `Bearer ${token}`)
      .send({
        quantity: '12 leaves'
      })
      .expect(200);

    expect(res.body.data.quantity).toBe('12 leaves');
  });

  it('should remove an ingredient from a cocktail', async () => {
    await request(app)
      .post(`/api/cocktail/${cocktailId}/ingredient`)
      .set('Authorization', `Bearer ${token}`)
      .send({
        ingredientId: mintId,
        quantity: '10 leaves'
      });

    await request(app)
      .delete(`/api/cocktail/${cocktailId}/ingredient/${mintId}`)
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    const res = await request(app)
      .get(`/api/cocktail/${cocktailId}`)
      .set('Authorization', `Bearer ${token}`);

    expect(res.body.data.ingredients).toHaveLength(0);
  });
});