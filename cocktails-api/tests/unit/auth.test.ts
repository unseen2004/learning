import { createJWT, hashPassword, comparePasswords } from '../../src/modules/auth';

describe('Auth Module', () => {
  const user = { id: 1, username: 'testuser' };

  beforeAll(() => {
    process.env.JWT_SECRET = 'test_secret';
  });

  it('should create a valid JWT token', () => {
    const token = createJWT(user);
    expect(typeof token).toBe('string');
    expect(token.split('.').length).toBe(3); // JWT has 3 parts
  });

  it('should hash passwords', async () => {
    const password = 'password123';
    const hash = await hashPassword(password);

    expect(hash).not.toBe(password);
    expect(hash.length).toBeGreaterThan(10);
  });

  it('should verify passwords correctly', async () => {
    const password = 'password123';
    const hash = await hashPassword(password);

    const isValid = await comparePasswords(password, hash);
    expect(isValid).toBe(true);

    const isInvalid = await comparePasswords('wrongpassword', hash);
    expect(isInvalid).toBe(false);
  });
});