import request from 'supertest';
import app from '../../src/server';
import { testUser } from '../fixtures/users';
import { mint, rum } from '../fixtures/ingredients';

describe('Ingredient API', () => {
  let token: string;

  beforeEach(async () => {
    const res = await request(app)
      .post('/user')
      .send(testUser);

    token = res.body.token;
  });

  it('should create a new ingredient', async () => {
    const res = await request(app)
      .post('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .field('name', mint.name)
      .field('description', mint.description)
      .field('alcohol', mint.alcohol)
      .expect(200);

    expect(res.body.data).toHaveProperty('id');
    expect(res.body.data.name).toBe(mint.name);
  });

  it('should get all ingredients for authenticated user', async () => {
    await request(app)
      .post('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .field('name', mint.name)
      .field('description', mint.description)
      .field('alcohol', mint.alcohol);

    const res = await request(app)
      .get('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    expect(Array.isArray(res.body.data)).toBe(true);
    expect(res.body.data.length).toBeGreaterThan(0);
  });

  it('should get a specific ingredient by ID', async () => {
    const createRes = await request(app)
      .post('/api/ingredient')
      .set('Authorization', `Bearer ${token}`)
      .field('name', mint.name)
      .field('description', mint.description)
      .field('alcohol', mint.alcohol);

    const ingredientId = createRes.body.data.id;

    const res = await request(app)
      .get(`/api/ingredient/${ingredientId}`)
      .set('Authorization', `Bearer ${token}`)
      .expect(200);

    expect(res.body.data.id).toBe(ingredientId);
    expect(res.body.data.name).toBe(mint.name);
  });
});