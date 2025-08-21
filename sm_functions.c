#include "sm_header.h"

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================
Product *productCache = NULL;
ProductIndex *indexCache = NULL;
int productCount = 0;
int indexCount = 0;
int nextProductID = 1;
int nextSaleID = 1;
int isLoggedIn = 0;
int isAdmin = 0;
char currentUser[30] = "";

// ============================================================================
// AUTHENTICATION SYSTEM
// ============================================================================

void initializeUsers(void) {
    FILE *file = fopen(USERS_FILE, "rb");
    if (file == NULL) {
        // Create default admin user
        file = fopen(USERS_FILE, "wb");
        if (file) {
            User defaultAdmin;
            strcpy(defaultAdmin.username, "admin");
            strcpy(defaultAdmin.password, "admin123");
            defaultAdmin.isAdmin = 1;
            
            fwrite(&defaultAdmin, sizeof(User), 1, file);
            fclose(file);
            printf("Default admin user created (username: admin, password: admin123)\n");
        }
    } else {
        fclose(file);
    }
}

int authenticateUser(char *username, char *password) {
    FILE *file = fopen(USERS_FILE, "rb");
    if (!file) return 0;
    
    User user;
    while (fread(&user, sizeof(User), 1, file)) {
        if (strcmp(user.username, username) == 0 && 
            strcmp(user.password, password) == 0) {
            fclose(file);
            strcpy(currentUser, username);
            isLoggedIn = 1;
            isAdmin = user.isAdmin;
            return 1;
        }
    }
    
    fclose(file);
    return 0;
}

void loginSystem(void) {
    char username[30], password[50];
    int attempts = 0;
    
    printf("\n=== LOGIN REQUIRED ===\n");
    
    while (attempts < 3 && !isLoggedIn) {
        printf("Username: ");
        scanf("%s", username);
        printf("Password: ");
        scanf("%s", password);
        
        if (authenticateUser(username, password)) {
            printf("Login successful! Welcome, %s\n", currentUser);
            if (isAdmin) {
                printf("Admin privileges granted.\n");
            }
        } else {
            attempts++;
            printf("Invalid credentials. Attempts remaining: %d\n", 3 - attempts);
        }
    }
    
    if (!isLoggedIn) {
        printf("Too many failed attempts. Exiting...\n");
        exit(1);
    }
}

void addUser(void) {
    if (!isAdmin) {
        printf("Access denied. Admin privileges required.\n");
        return;
    }
    
    User newUser;
    printf("\n=== ADD NEW USER ===\n");
    printf("Username: ");
    scanf("%s", newUser.username);
    printf("Password: ");
    scanf("%s", newUser.password);
    printf("Admin privileges? (1=Yes, 0=No): ");
    scanf("%d", &newUser.isAdmin);
    
    FILE *file = fopen(USERS_FILE, "ab");
    if (file) {
        fwrite(&newUser, sizeof(User), 1, file);
        fclose(file);
        printf("User added successfully!\n");
    }
}

// ============================================================================
// FILE MANAGEMENT & SYSTEM INITIALIZATION
// ============================================================================

void initializeSystem(void) {
    FILE *file;
    
    // Initialize products file if it doesn't exist
    file = fopen(PRODUCTS_FILE, "rb");
    if (file == NULL) {
        file = fopen(PRODUCTS_FILE, "wb");
        if (file) {
            printf("Created new products database.\n");
            fclose(file);
        }
    } else {
        fclose(file);
    }
    
    // Initialize sales file if it doesn't exist
    file = fopen(SALES_FILE, "rb");
    if (file == NULL) {
        file = fopen(SALES_FILE, "wb");
        if (file) {
            printf("Created new sales database.\n");
            fclose(file);
        }
    } else {
        fclose(file);
    }
    
    loadProductsToCache();
    loadIndexToCache();
    initializeUsers();
}

void loadProductsToCache(void) {
    FILE *file = fopen(PRODUCTS_FILE, "rb");
    if (!file) return;
    
    // Count products
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    productCount = fileSize / sizeof(Product);
    
    if (productCount > 0) {
        productCache = malloc(productCount * sizeof(Product));
        fseek(file, 0, SEEK_SET);
        fread(productCache, sizeof(Product), productCount, file);
        
        // Find next available ID
        for (int i = 0; i < productCount; i++) {
            if (productCache[i].productID >= nextProductID) {
                nextProductID = productCache[i].productID + 1;
            }
        }
    }
    
    fclose(file);
}

void saveProductsFromCache(void) {
    FILE *file = fopen(PRODUCTS_FILE, "wb");
    if (!file) {
        printf("Error saving products!\n");
        return;
    }
    
    if (productCount > 0) {
        fwrite(productCache, sizeof(Product), productCount, file);
    }
    
    fclose(file);
    updateProductIndex();
}

void loadIndexToCache(void) {
    FILE *file = fopen(INDEX_FILE, "rb");
    if (!file) {
        updateProductIndex();
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    indexCount = fileSize / sizeof(ProductIndex);
    
    if (indexCount > 0) {
        indexCache = malloc(indexCount * sizeof(ProductIndex));
        fseek(file, 0, SEEK_SET);
        fread(indexCache, sizeof(ProductIndex), indexCount, file);
    }
    
    fclose(file);
}

void updateProductIndex(void) {
    FILE *file = fopen(INDEX_FILE, "wb");
    if (!file) return;
    
    if (indexCache) free(indexCache);
    
    indexCount = 0;
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            indexCount++;
        }
    }
    
    if (indexCount > 0) {
        indexCache = malloc(indexCount * sizeof(ProductIndex));
        int idx = 0;
        
        for (int i = 0; i < productCount; i++) {
            if (productCache[i].isActive) {
                indexCache[idx].productID = productCache[i].productID;
                indexCache[idx].filePosition = i * sizeof(Product);
                idx++;
            }
        }
        
        fwrite(indexCache, sizeof(ProductIndex), indexCount, file);
    }
    
    fclose(file);
}

// ============================================================================
// BACKUP & RESTORE SYSTEM (Simplified for cross-platform compatibility)
// ============================================================================

void createBackup(void) {
    if (!isAdmin) {
        printf("Access denied. Admin privileges required.\n");
        return;
    }
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char backupName[100];
    
    sprintf(backupName, "backup_%04d%02d%02d_%02d%02d%02d", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    // Backup products
    char srcFile[100], destFile[100];
    sprintf(srcFile, "%s", PRODUCTS_FILE);
    sprintf(destFile, "%s_products.dat", backupName);
    
    FILE *src = fopen(srcFile, "rb");
    FILE *dest = fopen(destFile, "wb");
    
    if (src && dest) {
        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytes, dest);
        }
        fclose(src);
        fclose(dest);
    }
    
    // Backup sales
    sprintf(srcFile, "%s", SALES_FILE);
    sprintf(destFile, "%s_sales.dat", backupName);
    
    src = fopen(srcFile, "rb");
    dest = fopen(destFile, "wb");
    
    if (src && dest) {
        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytes, dest);
        }
        fclose(src);
        fclose(dest);
    }
    
    printf("Backup created: %s\n", backupName);
}

void restoreBackup(void) {
    if (!isAdmin) {
        printf("Access denied. Admin privileges required.\n");
        return;
    }
    
    char backupName[100];
    printf("\n=== RESTORE BACKUP ===\n");
    printf("Enter backup name (without extension): ");
    scanf("%s", backupName);
    
    char srcFile[100], destFile[100];
    
    // Restore products
    sprintf(srcFile, "%s_products.dat", backupName);
    sprintf(destFile, "%s", PRODUCTS_FILE);
    
    FILE *src = fopen(srcFile, "rb");
    FILE *dest = fopen(destFile, "wb");
    
    if (src && dest) {
        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytes, dest);
        }
        fclose(src);
        fclose(dest);
        
        // Restore sales
        sprintf(srcFile, "%s_sales.dat", backupName);
        sprintf(destFile, "%s", SALES_FILE);
        
        src = fopen(srcFile, "rb");
        dest = fopen(destFile, "wb");
        
        if (src && dest) {
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dest);
            }
            fclose(src);
            fclose(dest);
            
            printf("Backup restored successfully!\n");
            printf("Reloading data...\n");
            
            // Reload data
            if (productCache) free(productCache);
            if (indexCache) free(indexCache);
            productCache = NULL;
            indexCache = NULL;
            productCount = 0;
            indexCount = 0;
            
            loadProductsToCache();
            loadIndexToCache();
        } else {
            printf("Error: Could not restore sales data.\n");
        }
    } else {
        printf("Error: Backup '%s' not found.\n", backupName);
    }
}

// ============================================================================
// CSV EXPORT FUNCTIONS
// ============================================================================

void exportProductsToCSV(void) {
    char filename[100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    sprintf(filename, "products_export_%04d%02d%02d.csv", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error creating CSV file!\n");
        return;
    }
    
    // Write header
    fprintf(file, "Product ID,Name,Category,Unit Price,Stock,Discount,Status\n");
    
    // Write data
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            fprintf(file, "%d,\"%s\",\"%s\",%.2f,%d,%.1f,Active\n",
                    productCache[i].productID,
                    productCache[i].name,
                    productCache[i].category,
                    productCache[i].unitPrice,
                    productCache[i].quantityInStock,
                    productCache[i].discountPercent);
        }
    }
    
    fclose(file);
    printf("Products exported to: %s\n", filename);
}

void exportSalesToCSV(void) {
    char filename[100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    sprintf(filename, "sales_export_%04d%02d%02d.csv", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    FILE *csvFile = fopen(filename, "w");
    if (!csvFile) {
        printf("Error creating CSV file!\n");
        return;
    }
    
    // Write header
    fprintf(csvFile, "Sale ID,Product ID,Product Name,Quantity,Unit Price,Discount,Total,Date,Time\n");
    
    // Read sales data and write to CSV
    FILE *salesFile = fopen(SALES_FILE, "rb");
    if (salesFile) {
        Sale sale;
        while (fread(&sale, sizeof(Sale), 1, salesFile)) {
            // Find product name
            char productName[50] = "Unknown";
            for (int i = 0; i < productCount; i++) {
                if (productCache[i].productID == sale.productID) {
                    strcpy(productName, productCache[i].name);
                    break;
                }
            }
            
            fprintf(csvFile, "%d,%d,\"%s\",%d,%.2f,%.2f,%.2f,%s,%s\n",
                    sale.saleID,
                    sale.productID,
                    productName,
                    sale.quantitySold,
                    sale.unitPrice,
                    sale.discountApplied,
                    sale.totalPrice,
                    sale.date,
                    sale.time);
        }
        fclose(salesFile);
    }
    
    fclose(csvFile);
    printf("Sales exported to: %s\n", filename);
}

void exportLowStockToCSV(void) {
    char filename[100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    sprintf(filename, "low_stock_report_%04d%02d%02d.csv", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error creating CSV file!\n");
        return;
    }
    
    // Write header
    fprintf(file, "Product ID,Name,Category,Current Stock,Unit Price,Status\n");
    
    // Write low stock products
    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive && productCache[i].quantityInStock < LOW_STOCK_THRESHOLD) {
            fprintf(file, "%d,\"%s\",\"%s\",%d,%.2f,Low Stock\n",
                    productCache[i].productID,
                    productCache[i].name,
                    productCache[i].category,
                    productCache[i].quantityInStock,
                    productCache[i].unitPrice);
            found = 1;
        }
    }
    
    if (!found) {
        fprintf(file, "No products below stock threshold of %d units\n", LOW_STOCK_THRESHOLD);
    }
    
    fclose(file);
    printf("Low stock report exported to: %s\n", filename);
}

// ============================================================================
// INVENTORY MANAGEMENT (CRUD OPERATIONS)
// ============================================================================

void addProduct(void) {
    Product newProduct;
    
    printf("\n=== ADD NEW PRODUCT ===\n");
    newProduct.productID = nextProductID++;
    newProduct.isActive = 1;
    newProduct.discountPercent = 0.0;
    
    printf("Product ID (auto-generated): %d\n", newProduct.productID);
    
    printf("Enter product name: ");
    getchar(); // consume newline
    fgets(newProduct.name, sizeof(newProduct.name), stdin);
    newProduct.name[strcspn(newProduct.name, "\n")] = 0;
    
    printf("Enter category: ");
    fgets(newProduct.category, sizeof(newProduct.category), stdin);
    newProduct.category[strcspn(newProduct.category, "\n")] = 0;
    
    printf("Enter unit price: $");
    scanf("%f", &newProduct.unitPrice);
    
    printf("Enter quantity in stock: ");
    scanf("%d", &newProduct.quantityInStock);
    
    // Add to cache
    productCache = realloc(productCache, (productCount + 1) * sizeof(Product));
    productCache[productCount] = newProduct;
    productCount++;
    
    saveProductsFromCache();
    
    printf("Product added successfully!\n");
}

void viewAllProducts(void) {
    printf("\n=== PRODUCT INVENTORY ===\n");
    printf("%-5s %-25s %-15s %-10s %-8s %-8s %-8s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount", "Status");
    printf("================================================================================\n");
    
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            printf("%-5d %-25s %-15s $%-9.2f %-8d %-7.1f%% Active\n", 
                   productCache[i].productID,
                   productCache[i].name,
                   productCache[i].category,
                   productCache[i].unitPrice,
                   productCache[i].quantityInStock,
                   productCache[i].discountPercent);
        }
    }
    printf("\n");
}

int findProductByID(int productID) {
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].productID == productID && productCache[i].isActive) {
            return i;
        }
    }
    return -1;
}

void updateProduct(void) {
    int productID, choice, index;
    
    printf("\n=== UPDATE PRODUCT ===\n");
    printf("Enter product ID to update: ");
    scanf("%d", &productID);
    
    index = findProductByID(productID);
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }
    
    printf("Current product details:\n");
    printf("Name: %s\n", productCache[index].name);
    printf("Category: %s\n", productCache[index].category);
    printf("Price: $%.2f\n", productCache[index].unitPrice);
    printf("Stock: %d\n", productCache[index].quantityInStock);
    printf("Discount: %.1f%%\n", productCache[index].discountPercent);
    
    printf("\nWhat would you like to update?\n");
    printf("1. Name\n2. Category\n3. Price\n4. Stock\n5. Discount\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    getchar(); // consume newline
    
    switch (choice) {
        case 1:
            printf("Enter new name: ");
            fgets(productCache[index].name, sizeof(productCache[index].name), stdin);
            productCache[index].name[strcspn(productCache[index].name, "\n")] = 0;
            break;
        case 2:
            printf("Enter new category: ");
            fgets(productCache[index].category, sizeof(productCache[index].category), stdin);
            productCache[index].category[strcspn(productCache[index].category, "\n")] = 0;
            break;
        case 3:
            printf("Enter new price: $");
            scanf("%f", &productCache[index].unitPrice);
            break;
        case 4:
            printf("Enter new stock quantity: ");
            scanf("%d", &productCache[index].quantityInStock);
            break;
        case 5:
            printf("Enter discount percentage (0-100): ");
            scanf("%f", &productCache[index].discountPercent);
            if (productCache[index].discountPercent < 0) productCache[index].discountPercent = 0;
            if (productCache[index].discountPercent > 100) productCache[index].discountPercent = 100;
            break;
        default:
            printf("Invalid choice!\n");
            return;
    }
    
    saveProductsFromCache();
    printf("Product updated successfully!\n");
}

void deleteProduct(void) {
    int productID, index;
    
    printf("\n=== DELETE PRODUCT ===\n");
    printf("Enter product ID to delete: ");
    scanf("%d", &productID);
    
    index = findProductByID(productID);
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }
    
    printf("Are you sure you want to delete '%s'? (y/N): ", productCache[index].name);
    char confirm;
    scanf(" %c", &confirm);
    
    if (confirm == 'y' || confirm == 'Y') {
        productCache[index].isActive = 0; // Soft delete
        saveProductsFromCache();
        printf("Product deleted successfully!\n");
    } else {
        printf("Deletion cancelled.\n");
    }
}

// ============================================================================
// DISCOUNT & PROMOTION SYSTEM
// ============================================================================

void setProductDiscount(void) {
    int productID, index;
    float discount;
    
    printf("\n=== SET PRODUCT DISCOUNT ===\n");
    printf("Enter product ID: ");
    scanf("%d", &productID);
    
    index = findProductByID(productID);
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }
    
    printf("Current product: %s\n", productCache[index].name);
    printf("Current price: $%.2f\n", productCache[index].unitPrice);
    printf("Current discount: %.1f%%\n", productCache[index].discountPercent);
    
    printf("Enter new discount percentage (0-100): ");
    scanf("%f", &discount);
    
    if (discount < 0 || discount > 100) {
        printf("Invalid discount percentage!\n");
        return;
    }
    
    productCache[index].discountPercent = discount;
    saveProductsFromCache();
    
    float discountedPrice = productCache[index].unitPrice * (1 - discount / 100);
    printf("Discount set successfully!\n");
    printf("New effective price: $%.2f\n", discountedPrice);
}

void viewPromotions(void) {
    printf("\n=== ACTIVE PROMOTIONS ===\n");
    printf("%-5s %-25s %-10s %-10s %-10s\n", 
           "ID", "Name", "Price", "Discount", "Sale Price");
    printf("=================================================================\n");
    
    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive && productCache[i].discountPercent > 0) {
            float salePrice = productCache[i].unitPrice * (1 - productCache[i].discountPercent / 100);
            printf("%-5d %-25s $%-9.2f %-9.1f%% $%-9.2f\n", 
                   productCache[i].productID,
                   productCache[i].name,
                   productCache[i].unitPrice,
                   productCache[i].discountPercent,
                   salePrice);
            found = 1;
        }
    }
    
    if (!found) {
        printf("No active promotions.\n");
    }
}

// ============================================================================
// SALES MANAGEMENT
// ============================================================================

void getCurrentDateTime(char *dateStr, char *timeStr) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(dateStr, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(timeStr, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void processNewSale(void) {
    int productID, quantity, index;
    Sale newSale;
    
    printf("\n=== PROCESS NEW SALE ===\n");
    printf("Enter product ID: ");
    scanf("%d", &productID);
    
    index = findProductByID(productID);
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }
    
    printf("Product: %s\n", productCache[index].name);
    printf("Regular price: $%.2f\n", productCache[index].unitPrice);
    
    float effectivePrice = productCache[index].unitPrice;
    float discountAmount = 0;
    
    if (productCache[index].discountPercent > 0) {
        discountAmount = productCache[index].unitPrice * (productCache[index].discountPercent / 100);
        effectivePrice = productCache[index].unitPrice - discountAmount;
        printf("Discount: %.1f%% (-$%.2f)\n", productCache[index].discountPercent, discountAmount);
        printf("Sale price: $%.2f\n", effectivePrice);
    }
    
    printf("Available stock: %d\n", productCache[index].quantityInStock);
    
    printf("Enter quantity to sell: ");
    scanf("%d", &quantity);
    
    if (quantity <= 0) {
        printf("Invalid quantity!\n");
        return;
    }
    
    if (quantity > productCache[index].quantityInStock) {
        printf("Insufficient stock! Only %d available.\n", productCache[index].quantityInStock);
        return;
    }
    
    // Create sale record
    newSale.saleID = nextSaleID++;
    newSale.productID = productID;
    newSale.quantitySold = quantity;
    newSale.unitPrice = productCache[index].unitPrice;
    newSale.discountApplied = discountAmount * quantity;
    newSale.totalPrice = effectivePrice * quantity;
    getCurrentDateTime(newSale.date, newSale.time);
    
    // Update stock
    productCache[index].quantityInStock -= quantity;
    
    // Save sale to file
    FILE *salesFile = fopen(SALES_FILE, "ab");
    if (salesFile) {
        fwrite(&newSale, sizeof(Sale), 1, salesFile);
        fclose(salesFile);
    }
    
    saveProductsFromCache();
    
    printf("\n=== SALE COMPLETED ===\n");
    printf("Sale ID: %d\n", newSale.saleID);
    if (newSale.discountApplied > 0) {
        printf("Subtotal: $%.2f\n", newSale.unitPrice * quantity);
        printf("Discount: -$%.2f\n", newSale.discountApplied);
    }
    printf("Total: $%.2f\n", newSale.totalPrice);
    printf("Date: %s %s\n", newSale.date, newSale.time);
    printf("Remaining stock: %d\n", productCache[index].quantityInStock);
}

void viewAllSales(void) {
    FILE *file = fopen(SALES_FILE, "rb");
    if (!file) {
        printf("No sales data found.\n");
        return;
    }
    
    Sale sale;
    printf("\n=== SALES HISTORY ===\n");
    printf("%-8s %-10s %-8s %-10s %-10s %-12s %-10s\n", 
           "Sale ID", "Product ID", "Qty", "Unit Price", "Discount", "Total", "DateTime");
    printf("=============================================================================\n");
    
    while (fread(&sale, sizeof(Sale), 1, file)) {
        printf("%-8d %-10d %-8d $%-9.2f $%-9.2f $%-11.2f %s %s\n", 
               sale.saleID, sale.productID, sale.quantitySold, 
               sale.unitPrice, sale.discountApplied, sale.totalPrice, 
               sale.date, sale.time);
    }
    
    fclose(file);
}

// ============================================================================
// SEARCH & SORT FUNCTIONS
// ============================================================================

void searchProductByName(void) {
    char searchName[50];
    int found = 0;
    
    printf("\n=== SEARCH BY NAME ===\n");
    printf("Enter product name (or partial name): ");
    getchar(); // consume newline
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = 0;
    
    // Convert to lowercase for case-insensitive search
    for (int i = 0; searchName[i]; i++) {
        searchName[i] = tolower(searchName[i]);
    }
    
    printf("Search results:\n");
    printf("%-5s %-25s %-15s %-10s %-8s %-8s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount");
    printf("========================================================================\n");
    
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            char productName[50];
            strcpy(productName, productCache[i].name);
            
            // Convert to lowercase for comparison
            for (int j = 0; productName[j]; j++) {
                productName[j] = tolower(productName[j]);
            }
            
            if (strstr(productName, searchName) != NULL) {
                printf("%-5d %-25s %-15s $%-9.2f %-8d %-7.1f%%\n", 
                       productCache[i].productID,
                       productCache[i].name,
                       productCache[i].category,
                       productCache[i].unitPrice,
                       productCache[i].quantityInStock,
                       productCache[i].discountPercent);
                found = 1;
            }
        }
    }
    
    if (!found) {
        printf("No products found matching '%s'\n", searchName);
    }
}

void searchByCategory(void) {
    char searchCategory[30];
    int found = 0;
    
    printf("\n=== SEARCH BY CATEGORY ===\n");
    printf("Enter category: ");
    getchar(); // consume newline
    fgets(searchCategory, sizeof(searchCategory), stdin);
    searchCategory[strcspn(searchCategory, "\n")] = 0;
    
    // Convert to lowercase for case-insensitive search
    for (int i = 0; searchCategory[i]; i++) {
        searchCategory[i] = tolower(searchCategory[i]);
    }
    
    printf("Products in category '%s':\n", searchCategory);
    printf("%-5s %-25s %-10s %-8s %-8s\n", 
           "ID", "Name", "Price", "Stock", "Discount");
    printf("===========================================================\n");
    
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            char category[30];
            strcpy(category, productCache[i].category);
            
            // Convert to lowercase for comparison
            for (int j = 0; category[j]; j++) {
                category[j] = tolower(category[j]);
            }
            
            if (strstr(category, searchCategory) != NULL) {
                printf("%-5d %-25s $%-9.2f %-8d %-7.1f%%\n", 
                       productCache[i].productID,
                       productCache[i].name,
                       productCache[i].unitPrice,
                       productCache[i].quantityInStock,
                       productCache[i].discountPercent);
                found = 1;
            }
        }
    }
    
    if (!found) {
        printf("No products found in category '%s'\n", searchCategory);
    }
}

/* void sortProductsByPrice(void) {
    int choice;
    printf("\n=== SORT BY PRICE ===\n");
    printf("1. Ascending (Low to High)\n");
    printf("2. Descending (High to Low)\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    
    // Simple bubble sort
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            if (productCache[j].isActive && productCache[j+1].isActive) {
                int shouldSwap = 0;
                
                if (choice == 1) { // Ascending
                    shouldSwap = productCache[j].unitPrice > productCache[j+1].unitPrice;
                } else { // Descending
                    shouldSwap = productCache[j].unitPrice < productCache[j+1].unitPrice;
                }
                
                if (shouldSwap) {
                    Product temp = productCache[j];
                    productCache[j] = productCache[j+1];
                    productCache[j+1] = temp;
                }
            }
        }
    }
    
    printf("\nProducts sorted by price:\n");
    viewAllProducts();
}
 */


// Main sorting menu
void sortingMenu() {
    int choice;
    
    while (1) {
        system("cls");
        printf("\n====================================\n");
        printf("|         SORTING OPTIONS          |\n");
        printf("====================================\n");
        printf("1. View Products by ID (Original Order)\n");
        printf("2. View Products by Price (Low to High)\n");
        printf("3. View Products by Price (High to Low)\n");
        printf("4. View Products by Name (A-Z)\n");
        printf("5. View Products by Stock (Low to High)\n");
        printf("6. Permanently Sort by ID\n");
        printf("7. Permanently Sort by Price\n");
        printf("0. Back to Inventory Menu\n");
        printf("\n===================================\n");
        printf("Enter your choice: ");
        
        scanf("%d", &choice);
        getchar(); // consume newline
        
        switch (choice) {
            case 1: viewProductsSortedByID(); break;
            case 2: viewProductsSortedByPrice(); break;
            case 3: viewProductsSortedByPriceDesc(); break;
            case 4: viewProductsSortedByName(); break;
            case 5: viewProductsSortedByStock(); break;
            case 6: permanentSortByID(); break;
            case 7: permanentSortByPrice(); break;
            case 0: return;
            default: 
                printf("\n❌ Invalid choice! Please try again.\n");
                printf("Press any key to continue...");
                getchar();
        }
    }
}

// View products sorted by ID (temporary sort)
void viewProductsSortedByID() {
    printf("\n====================================\n");
    printf("|    PRODUCTS SORTED BY ID         |\n");
    printf("====================================\n");
    
    if (productCount == 0) {
        printf("📦 No products available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Create a temporary copy for sorting
    Product tempProducts[MAX_PRODUCTS];
    for (int i = 0; i < productCount; i++) {
        tempProducts[i] = productCache[i];
    }
    
    // Sort the temporary array by ID
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            if (tempProducts[j].productID > tempProducts[j + 1].productID) {
                Product temp = tempProducts[j];
                tempProducts[j] = tempProducts[j + 1];
                tempProducts[j + 1] = temp;
            }
        }
    }
    
    // Display sorted products
    printf("\n%-5s %-20s %-15s %-12s %-8s %-10s\n", 
           "ID", "Name", "Category", "unitPrice", "Stock", "Discount%");
    printf("─────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < productCount; i++) {
        double finalPrice = tempProducts[i].unitPrice * (1 - tempProducts[i].discountPercent / 100.0);
        printf("%-5d %-20s %-15s $%-11.2f %-8d %-10.1f\n",
               tempProducts[i].productID,
               tempProducts[i].name,
               tempProducts[i].category,
               finalPrice,
               tempProducts[i].quantityInStock,
               tempProducts[i].discountPercent);
    }
    
    printf("\n💡 Note: This is a temporary view. Original data order unchanged.\n");
    printf("Total Products: %d\n", productCount);
    printf("\nPress any key to continue...");
    getchar();
}

// View products sorted by price (low to high)
void viewProductsSortedByPrice() {
    printf("\n====================================\n");
    printf("|   PRODUCTS BY PRICE (LOW→HIGH)   \n");
    printf("====================================\n");
    
    if (productCount == 0) {
        printf("📦 No products available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Create temporary copy for sorting
    Product tempProducts[MAX_PRODUCTS];
    for (int i = 0; i < productCount; i++) {
        tempProducts[i] = productCache[i];
    }
    
    // Sort by price (low to high)
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            double price1 = tempProducts[j].unitPrice * (1 - tempProducts[j].discountPercent / 100.0);
            double price2 = tempProducts[j + 1].unitPrice * (1 - tempProducts[j + 1].discountPercent / 100.0);
            
            if (price1 > price2) {
                Product temp = tempProducts[j];
                tempProducts[j] = tempProducts[j + 1];
                tempProducts[j + 1] = temp;
            }
        }
    }
    
    // Display
    printf("\n%-5s %-20s %-15s %-12s %-8s %-10s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount%");
    printf("─────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < productCount; i++) {
        double finalPrice = tempProducts[i].unitPrice * (1 - tempProducts[i].discountPercent / 100.0);
        printf("%-5d %-20s %-15s $%-11.2f %-8d %-10.1f\n",
               tempProducts[i].productID, tempProducts[i].name, tempProducts[i].category,
               finalPrice, tempProducts[i].quantityInStock, tempProducts[i].discountPercent);
    }
    
    printf("\n💡 Note: Prices shown include discounts. Original data unchanged.\n");
    printf("Press any key to continue...");
    getchar();
}

// View products sorted by price (high to low)
void viewProductsSortedByPriceDesc() {
    printf("\n====================================\n");
    printf("   PRODUCTS BY PRICE (HIGH→LOW)   \n");
    printf("====================================\n");
    
    if (productCount == 0) {
        printf("📦 No products available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Create temporary copy and sort descending
    Product tempProducts[MAX_PRODUCTS];
    for (int i = 0; i < productCount; i++) {
        tempProducts[i] = productCache[i];
    }
    
    // Sort by price (high to low)
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            double price1 = tempProducts[j].unitPrice * (1 - tempProducts[j].discountPercent / 100.0);
            double price2 = tempProducts[j + 1].unitPrice * (1 - tempProducts[j + 1].discountPercent / 100.0);
            
            if (price1 < price2) {
                Product temp = tempProducts[j];
                tempProducts[j] = tempProducts[j + 1];
                tempProducts[j + 1] = temp;
            }
        }
    }
    
    // Display
    printf("\n%-5s %-20s %-15s %-12s %-8s %-10s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount%");
    printf("─────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < productCount; i++) {
        double finalPrice = tempProducts[i].unitPrice * (1 - tempProducts[i].discountPercent / 100.0);
        printf("%-5d %-20s %-15s $%-11.2f %-8d %-10.1f\n",
               tempProducts[i].productID, tempProducts[i].name, tempProducts[i].category,
               finalPrice, tempProducts[i].quantityInStock, tempProducts[i].discountPercent);
    }
    
    printf("\n💰 Showing highest priced items first. Original data unchanged.\n");
    printf("Press any key to continue...");
    getchar();
}

// View products sorted by name (A-Z)
void viewProductsSortedByName() {
    printf("\n====================================\n");
    printf("    PRODUCTS SORTED BY NAME       \n");
    printf("====================================\n");
    
    if (productCount == 0) {
        printf("📦 No products available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Create temporary copy and sort by name
    Product tempProducts[MAX_PRODUCTS];
    for (int i = 0; i < productCount; i++) {
        tempProducts[i] = productCache[i];
    }
    
    // Sort by name (A-Z)
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            if (strcmp(tempProducts[j].name, tempProducts[j + 1].name) > 0) {
                Product temp = tempProducts[j];
                tempProducts[j] = tempProducts[j + 1];
                tempProducts[j + 1] = temp;
            }
        }
    }
    
    // Display
    printf("\n%-5s %-20s %-15s %-12s %-8s %-10s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount%");
    printf("─────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < productCount; i++) {
        double finalPrice = tempProducts[i].unitPrice * (1 - tempProducts[i].discountPercent / 100.0);
        printf("%-5d %-20s %-15s $%-11.2f %-8d %-10.1f\n",
               tempProducts[i].productID, tempProducts[i].name, tempProducts[i].category,
               finalPrice, tempProducts[i].quantityInStock, tempProducts[i].discountPercent);
    }
    
    printf("\n🔤 Sorted alphabetically A-Z. Original data unchanged.\n");
    printf("Press any key to continue...");
    getchar();
}

// View products sorted by stock (low to high)
void viewProductsSortedByStock() {
    printf("\n====================================\n");
    printf(" | PRODUCTS BY STOCK (LOW→HIGH)   |\n");
    printf("====================================\n");
    
    if (productCount == 0) {
        printf("📦 No products available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    // Create temporary copy and sort by stock
    Product tempProducts[MAX_PRODUCTS];
    for (int i = 0; i < productCount; i++) {
        tempProducts[i] = productCache[i];
    }
    
    // Sort by stock (low to high)
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            if (tempProducts[j].quantityInStock > tempProducts[j + 1].quantityInStock) {
                Product temp = tempProducts[j];
                tempProducts[j] = tempProducts[j + 1];
                tempProducts[j + 1] = temp;
            }
        }
    }
    
    // Display with stock level indicators
    printf("\n%-5s %-20s %-15s %-12s %-8s %-10s %-10s\n", 
           "ID", "Name", "Category", "Price", "Stock", "Discount%", "Status");
    printf("─────────────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < productCount; i++) {
        double finalPrice = tempProducts[i].unitPrice * (1 - tempProducts[i].discountPercent / 100.0);
        
        // Determine stock status
        char status[20];
        if (tempProducts[i].quantityInStock == 0) {
            strcpy(status, "❌ OUT");
        } else if (tempProducts[i].quantityInStock <= 10) {
            strcpy(status, "⚠️ LOW");
        } else if (tempProducts[i].quantityInStock <= 50) {
            strcpy(status, "✅ OK");
        } else {
            strcpy(status, "💚 HIGH");
        }
        
        printf("%-5d %-20s %-15s $%-11.2f %-8d %-10.1f %-10s\n",
               tempProducts[i].productID, tempProducts[i].name, tempProducts[i].category,
               finalPrice, tempProducts[i].quantityInStock, tempProducts[i].discountPercent, status);
    }
    
    printf("\n📊 Sorted by stock level (lowest first). Check for reorder needs!\n");
    printf("Press any key to continue...");
    getchar();
}

// Permanently sort by ID
void permanentSortByID() {
    printf("\n====================================\n");
    printf("|     PERMANENT SORT BY ID         |\n");
    printf("====================================\n");
    
    if (productCount <= 1) {
        printf("📦 No products to sort or only one product available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    printf("⚠️  WARNING: This will permanently change your data order!\n");
    printf("📋 Current product count: %d\n", productCount);
    printf("💾 This change will be saved to your data file.\n");
    printf("\nAre you sure you want to permanently sort by ID? (y/N): ");
    
    char confirm;
    scanf("%c", &confirm);
    getchar();
    
    if (confirm == 'y' || confirm == 'Y') {
        printf("\n🔄 Sorting products by ID...\n");
        
        // Sort by product ID
        for (int i = 0; i < productCount - 1; i++) {
            for (int j = 0; j < productCount - i - 1; j++) {
                if (productCache[j].productID > productCache[j + 1].productID) {
                    // Swap products
                    Product temp = productCache[j];
                    productCache[j] = productCache[j + 1];
                    productCache[j + 1] = temp;
                }
            }
        }
        
        // Save to file
        saveProductsFromCache();
        
        printf("✅ Products permanently sorted by ID and saved!\n");
    } else {
        printf("❌ Sort operation cancelled.\n");
    }
    
    printf("Press any key to continue...");
    getchar();
}

// Permanently sort by price
void permanentSortByPrice() {
    printf("\n====================================\n");
    printf("|    PERMANENT SORT BY PRICE       |\n");
    printf("====================================\n");
    
    if (productCount <= 1) {
        printf("📦 No products to sort or only one product available.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    printf("⚠️  WARNING: This will permanently change your data order!\n");
    printf("📋 Current product count: %d\n", productCount);
    printf("💾 This change will be saved to your data file.\n");
    printf("\n1. Sort Low to High\n");
    printf("2. Sort High to Low\n");
    printf("0. Cancel\n");
    printf("\nEnter your choice: ");
    
    int sortChoice;
    scanf("%d", &sortChoice);
    getchar();
    
    if (sortChoice == 0) {
        printf("❌ Sort operation cancelled.\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    if (sortChoice != 1 && sortChoice != 2) {
        printf("❌ Invalid choice!\n");
        printf("Press any key to continue...");
        getchar();
        return;
    }
    
    printf("\n🔄 Sorting products by price...\n");
    
    // Sort by price
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            double price1 = productCache[j].unitPrice * (1 - productCache[j].discountPercent / 100.0);
            double price2 = productCache[j + 1].unitPrice * (1 - productCache[j + 1].discountPercent / 100.0);
            
            int shouldSwap = (sortChoice == 1) ? (price1 > price2) : (price1 < price2);
            
            if (shouldSwap) {
                Product temp = productCache[j];
                productCache[j] = productCache[j + 1];
                productCache[j + 1] = temp;
            }
        }
    }
    
    // Save to file
    saveProductsFromCache();
    
    printf("✅ Products permanently sorted by price %s and saved!\n", 
           (sortChoice == 1) ? "(Low→High)" : "(High→Low)");
    printf("Press any key to continue...");
    getchar();
}



// ============================================================================
// REPORTS & ANALYTICS
// ============================================================================

void lowStockReport(void) {
    int threshold = LOW_STOCK_THRESHOLD;
    int found = 0;
    
    printf("\n=== LOW STOCK REPORT ===\n");
    printf("Products with stock below %d:\n", threshold);
    printf("%-5s %-25s %-15s %-10s %-8s\n", 
           "ID", "Name", "Category", "Price", "Stock");
    printf("=====================================================================\n");
    
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive && productCache[i].quantityInStock < threshold) {
            printf("%-5d %-25s %-15s $%-9.2f %-8d\n", 
                   productCache[i].productID,
                   productCache[i].name,
                   productCache[i].category,
                   productCache[i].unitPrice,
                   productCache[i].quantityInStock);
            found = 1;
        }
    }
    
    if (!found) {
        printf("All products have sufficient stock!\n");
    }
    
    printf("\nExport to CSV? (y/N): ");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        exportLowStockToCSV();
    }
}

void dailySalesReport(void) {
    char targetDate[11];
    float totalRevenue = 0;
    float totalDiscount = 0;
    int totalSales = 0;
    
    printf("\n=== DAILY SALES REPORT ===\n");
    printf("Enter date (YYYY-MM-DD) or 'today': ");
    scanf("%s", targetDate);
    
    if (strcmp(targetDate, "today") == 0) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        sprintf(targetDate, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    }
    
    FILE *file = fopen(SALES_FILE, "rb");
    if (!file) {
        printf("No sales data found.\n");
        return;
    }
    
    Sale sale;
    printf("\nSales for %s:\n", targetDate);
    printf("%-8s %-10s %-8s %-10s %-10s %-10s %-10s\n", 
           "Sale ID", "Product ID", "Qty", "Unit Price", "Discount", "Total", "Time");
    printf("=================================================================================\n");
    
    while (fread(&sale, sizeof(Sale), 1, file)) {
        if (strcmp(sale.date, targetDate) == 0) {
            printf("%-8d %-10d %-8d $%-9.2f $%-9.2f $%-9.2f %-10s\n", 
                   sale.saleID, sale.productID, sale.quantitySold, 
                   sale.unitPrice, sale.discountApplied, sale.totalPrice, sale.time);
            totalRevenue += sale.totalPrice;
            totalDiscount += sale.discountApplied;
            totalSales++;
        }
    }
    
    printf("=================================================================================\n");
    printf("Total Sales: %d\n", totalSales);
    printf("Total Discount Given: $%.2f\n", totalDiscount);
    printf("Total Revenue: $%.2f\n", totalRevenue);
    printf("Gross Revenue (before discounts): $%.2f\n", totalRevenue + totalDiscount);
    
    fclose(file);
}

void monthlyReport(void) {
    char targetMonth[8]; // YYYY-MM format
    float totalRevenue = 0;
    float totalDiscount = 0;
    int totalSales = 0;
    
    printf("\n=== MONTHLY REPORT ===\n");
    printf("Enter month (YYYY-MM): ");
    scanf("%s", targetMonth);
    
    FILE *file = fopen(SALES_FILE, "rb");
    if (!file) {
        printf("No sales data found.\n");
        return;
    }
    
    Sale sale;
    while (fread(&sale, sizeof(Sale), 1, file)) {
        if (strncmp(sale.date, targetMonth, 7) == 0) {
            totalRevenue += sale.totalPrice;
            totalDiscount += sale.discountApplied;
            totalSales++;
        }
    }
    
    printf("\n=== MONTHLY SUMMARY FOR %s ===\n", targetMonth);
    printf("Total Transactions: %d\n", totalSales);
    printf("Total Discount Given: $%.2f\n", totalDiscount);
    printf("Net Revenue: $%.2f\n", totalRevenue);
    printf("Gross Revenue: $%.2f\n", totalRevenue + totalDiscount);
    printf("Average Transaction: $%.2f\n", totalSales > 0 ? totalRevenue / totalSales : 0);
    
    fclose(file);
}

void bestSellingProducts(void) {
    typedef struct {
        int productID;
        int totalSold;
        float totalRevenue;
    } ProductSales;
    
    ProductSales sales[1000]; // Assuming max 1000 products
    int salesCount = 0;
    
    // Initialize sales data
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            sales[salesCount].productID = productCache[i].productID;
            sales[salesCount].totalSold = 0;
            sales[salesCount].totalRevenue = 0;
            salesCount++;
        }
    }
    
    // Calculate sales from file
    FILE *file = fopen(SALES_FILE, "rb");
    if (file) {
        Sale sale;
        while (fread(&sale, sizeof(Sale), 1, file)) {
            for (int i = 0; i < salesCount; i++) {
                if (sales[i].productID == sale.productID) {
                    sales[i].totalSold += sale.quantitySold;
                    sales[i].totalRevenue += sale.totalPrice;
                    break;
                }
            }
        }
        fclose(file);
    }
    
    // Sort by quantity sold (descending)
    for (int i = 0; i < salesCount - 1; i++) {
        for (int j = 0; j < salesCount - i - 1; j++) {
            if (sales[j].totalSold < sales[j+1].totalSold) {
                ProductSales temp = sales[j];
                sales[j] = sales[j+1];
                sales[j+1] = temp;
            }
        }
    }
    
    printf("\n=== BEST SELLING PRODUCTS ===\n");
    printf("%-5s %-25s %-15s %-12s %-12s\n", 
           "Rank", "Product Name", "Category", "Units Sold", "Revenue");
    printf("=======================================================================\n");
    
    int rank = 1;
    for (int i = 0; i < salesCount && rank <= 10; i++) {
        if (sales[i].totalSold > 0) {
            // Find product details
            for (int j = 0; j < productCount; j++) {
                if (productCache[j].productID == sales[i].productID) {
                    printf("%-5d %-25s %-15s %-12d $%-11.2f\n", 
                           rank++,
                           productCache[j].name,
                           productCache[j].category,
                           sales[i].totalSold,
                           sales[i].totalRevenue);
                    break;
                }
            }
        }
    }
}

void dateRangeReport(void) {
    char startDate[11], endDate[11];
    float totalRevenue = 0;
    float totalDiscount = 0;
    int totalSales = 0;
    
    printf("\n=== DATE RANGE REPORT ===\n");
    printf("Enter start date (YYYY-MM-DD): ");
    scanf("%s", startDate);
    printf("Enter end date (YYYY-MM-DD): ");
    scanf("%s", endDate);
    
    FILE *file = fopen(SALES_FILE, "rb");
    if (!file) {
        printf("No sales data found.\n");
        return;
    }
    
    Sale sale;
    printf("\nSales from %s to %s:\n", startDate, endDate);
    printf("%-8s %-12s %-8s %-10s %-10s %-10s\n", 
           "Sale ID", "Date", "Qty", "Unit Price", "Discount", "Total");
    printf("================================================================\n");
    
    while (fread(&sale, sizeof(Sale), 1, file)) {
        if (strcmp(sale.date, startDate) >= 0 && strcmp(sale.date, endDate) <= 0) {
            printf("%-8d %-12s %-8d $%-9.2f $%-9.2f $%-9.2f\n", 
                   sale.saleID, sale.date, sale.quantitySold, 
                   sale.unitPrice, sale.discountApplied, sale.totalPrice);
            totalRevenue += sale.totalPrice;
            totalDiscount += sale.discountApplied;
            totalSales++;
        }
    }
    
    printf("================================================================\n");
    printf("Summary for period %s to %s:\n", startDate, endDate);
    printf("Total Sales: %d\n", totalSales);
    printf("Total Discount: $%.2f\n", totalDiscount);
    printf("Net Revenue: $%.2f\n", totalRevenue);
    printf("Average Daily Sales: $%.2f\n", totalRevenue / ((float)totalSales > 0 ? totalSales : 1));
    
    fclose(file);
}

void categoryReport(void) {
    printf("\n=== CATEGORY ANALYSIS ===\n");
    
    // Get unique categories
    char categories[100][30];
    int categoryCount = 0;
    
    for (int i = 0; i < productCount; i++) {
        if (productCache[i].isActive) {
            int exists = 0;
            for (int j = 0; j < categoryCount; j++) {
                if (strcmp(categories[j], productCache[i].category) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists && categoryCount < 100) {
                strcpy(categories[categoryCount], productCache[i].category);
                categoryCount++;
            }
        }
    }
    
    printf("%-20s %-12s %-15s %-15s\n", "Category", "Products", "Total Stock", "Avg Price");
    printf("=================================================================\n");
    
    for (int i = 0; i < categoryCount; i++) {
        int productCount_cat = 0;
        int totalStock = 0;
        float totalPrice = 0;
        
        for (int j = 0; j < productCount; j++) {
            if (productCache[j].isActive && strcmp(productCache[j].category, categories[i]) == 0) {
                productCount_cat++;
                totalStock += productCache[j].quantityInStock;
                totalPrice += productCache[j].unitPrice;
            }
        }
        
        float avgPrice = productCount_cat > 0 ? totalPrice / productCount_cat : 0;
        printf("%-20s %-12d %-15d $%-14.2f\n", 
               categories[i], productCount_cat, totalStock, avgPrice);
    }
}